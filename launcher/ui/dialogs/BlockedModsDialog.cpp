// SPDX-FileCopyrightText: 2022 Sefa Eyeoglu <contact@scrumplex.net>
// SPDX-FileCopyrightText: 2022 Rachel Powers <508861+Ryex@users.noreply.github.com>
// SPDX-FileCopyrightText: 2022 kumquat-ir <66188216+kumquat-ir@users.noreply.github.com>

// SPDX-License-Identifier: GPL-3.0-only

/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
 *  Copyright (C) 2022 Rachel Powers <508861+Ryex@users.noreply.github.com>
 *  Copyright (C) 2022 kumquat-ir <66188216+kumquat-ir@users.noreply.github.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "BlockedModsDialog.h"
#include "ui_BlockedModsDialog.h"

#include "Application.h"
#include "settings/SettingsObject.h"
#include "modplatform/helpers/HashUtils.h"

#include <QDebug>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QDirIterator>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMimeData>
#include <QPushButton>
#include <QStandardPaths>
#include <QTimer>

BlockedModsDialog::BlockedModsDialog(QWidget* parent, const QString& title, const QString& text, QList<BlockedMod>& mods, QString hash_type)
    : QDialog(parent), ui(new Ui::BlockedModsDialog), m_mods(mods), m_hashType(hash_type)
{
    m_hashingTask = shared_qobject_ptr<ConcurrentTask>(
        new ConcurrentTask("MakeHashesTask", APPLICATION->settings()->get("NumberOfConcurrentTasks").toInt()));
    connect(m_hashingTask.get(), &Task::finished, this, &BlockedModsDialog::hashTaskFinished);

    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));
    connect(ui->openMissingButton, &QPushButton::clicked, this, [this]() { openAll(true); });
    connect(ui->downloadFolderButton, &QPushButton::clicked, this, &BlockedModsDialog::addDownloadFolder);

    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &BlockedModsDialog::directoryChanged);

    qDebug() << "[Blocked Mods Dialog] Mods List:" << mods;

    QTimer::singleShot(0, this, [this] {
        setupWatch();
        scanPaths();
        update();
    });

    this->setWindowTitle(title);
    ui->labelDescription->setText(text);

    connect(ui->textBrowserWatched, &QTextBrowser::anchorClicked, this, [](const QUrl url) { QDesktopServices::openUrl(url); });

    setAcceptDrops(true);

    update();
}

BlockedModsDialog::~BlockedModsDialog()
{
    delete ui;
}

void BlockedModsDialog::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void BlockedModsDialog::dropEvent(QDropEvent* e)
{
    for (QUrl& url : e->mimeData()->urls()) {
        if (url.scheme().isEmpty()) {

            url.setScheme("file");
        }

        if (!url.isLocalFile()) {

            continue;
        }

        QString filePath = url.toLocalFile();
        qDebug() << "[Blocked Mods Dialog] Dropped file:" << filePath;
        addHashTask(filePath);

        QFileInfo file = QFileInfo(filePath);
        QString path = file.dir().absolutePath();
        qDebug() << "[Blocked Mods Dialog] Adding watch path:" << path;
        m_watcher.addPath(path);
    }
    scanPaths();
    update();
}

void BlockedModsDialog::done(int r)
{
    QDialog::done(r);
    disconnect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &BlockedModsDialog::directoryChanged);
}

void BlockedModsDialog::openAll(bool missingOnly)
{
    for (auto& mod : m_mods) {
        if (!missingOnly || !mod.matched) {
            QDesktopServices::openUrl(mod.websiteUrl);
        }
    }
}

void BlockedModsDialog::addDownloadFolder()
{
    QString dir =
        QFileDialog::getExistingDirectory(this, tr("Select directory where you downloaded the mods"),
                                          QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), QFileDialog::ShowDirsOnly);
    qDebug() << "[Blocked Mods Dialog] Adding watch path:" << dir;
    m_watcher.addPath(dir);
    scanPath(dir, true);
    update();
}

void BlockedModsDialog::update()
{
    QString text;
    QString span;

    for (auto& mod : m_mods) {
        if (mod.matched) {

            span = QString(tr("<span style=\"color:green\"> &#x2714; Found at %1 </span>")).arg(mod.localPath);
        } else {

            span = QString(tr("<span style=\"color:red\"> &#x2718; Not Found </span>"));
        }
        text += QString(tr("%1: <a href='%2'>%2</a> <p>Hash: %3 %4</p> <br/>")).arg(mod.name, mod.websiteUrl, mod.hash, span);
    }

    ui->textBrowserModsListing->setText(text);

    QString watching;
    for (auto& dir : m_watcher.directories()) {
        QUrl fileURL = QUrl::fromLocalFile(dir);
        watching += QString("<a href=\"%1\">%2</a><br/>").arg(fileURL.toString(), dir);
    }

    ui->textBrowserWatched->setText(watching);

    if (allModsMatched()) {
        ui->labelModsFound->setText("<span style=\"color:green\">✔</span>" + tr("All mods found"));
        ui->openMissingButton->setDisabled(true);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));
    } else {
        ui->labelModsFound->setText(tr("Please download the missing mods."));
        ui->openMissingButton->setDisabled(false);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Skip"));
    }
}

void BlockedModsDialog::directoryChanged(QString path)
{
    qDebug() << "[Blocked Mods Dialog] Directory changed:" << path;
    validateMatchedMods();
    scanPath(path, true);
}

void BlockedModsDialog::setupWatch()
{
    const QString downloadsFolder = APPLICATION->settings()->get("DownloadsDir").toString();
    const QString modsFolder = APPLICATION->settings()->get("CentralModsDir").toString();
    const bool downloadsFolderWatchRecursive = APPLICATION->settings()->get("DownloadsDirWatchRecursive").toBool();
    watchPath(downloadsFolder, downloadsFolderWatchRecursive);
    watchPath(modsFolder, true);
}

void BlockedModsDialog::watchPath(QString path, bool watch_recursive)
{
    auto to_watch = QFileInfo(path);
    if (!to_watch.isReadable()) {
        qWarning() << "[Blocked Mods Dialog] Failed to add Watch Path (unable to read):" << path;
        return;
    }
    auto to_watch_path = to_watch.canonicalFilePath();
    if (m_watcher.directories().contains(to_watch_path))
        return;

    qDebug() << "[Blocked Mods Dialog] Adding Watch Path:" << path;
    m_watcher.addPath(to_watch_path);

    if (!to_watch.isDir() || !watch_recursive)
        return;

    QDirIterator it(to_watch_path, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        QString watch_dir = QDir(it.next()).canonicalPath();

        watchPath(watch_dir, watch_recursive);
    }
}

void BlockedModsDialog::scanPaths()
{
    for (auto& dir : m_watcher.directories()) {
        scanPath(dir, false);
    }
    runHashTask();
}

void BlockedModsDialog::scanPath(QString path, bool start_task)
{
    QDir scan_dir(path);
    QDirIterator scan_it(path, QDir::Filter::Files | QDir::Filter::Hidden, QDirIterator::NoIteratorFlags);
    while (scan_it.hasNext()) {
        QString file = scan_it.next();

        if (!checkValidPath(file)) {
            continue;
        }

        addHashTask(file);
    }

    if (start_task) {
        runHashTask();
    }
}

void BlockedModsDialog::addHashTask(QString path)
{
    qDebug() << "[Blocked Mods Dialog] adding a Hash task for" << path << "to the pending set.";
    m_pendingHashPaths.insert(path);
}

void BlockedModsDialog::buildHashTask(QString path)
{
    auto hash_task = Hashing::createHasher(path, m_hashType);

    qDebug() << "[Blocked Mods Dialog] Creating Hash task for path:" << path;

    connect(hash_task.get(), &Task::succeeded, this, [this, hash_task, path] { checkMatchHash(hash_task->getResult(), path); });
    connect(hash_task.get(), &Task::failed, this, [path] { qDebug() << "Failed to hash path:" << path; });

    m_hashingTask->addTask(hash_task);
}

void BlockedModsDialog::checkMatchHash(QString hash, QString path)
{
    bool match = false;

    qDebug() << "[Blocked Mods Dialog] Checking for match on hash:" << hash << "| From path:" << path;

    auto downloadDir = QFileInfo(APPLICATION->settings()->get("DownloadsDir").toString()).absoluteFilePath();
    auto moveFiles = APPLICATION->settings()->get("MoveModsFromDownloadsDir").toBool();
    for (auto& mod : m_mods) {
        if (mod.matched) {
            continue;
        }
        if (mod.hash.compare(hash, Qt::CaseInsensitive) == 0) {
            mod.matched = true;
            mod.localPath = path;
            if (moveFiles) {
                mod.move = QFileInfo(path).absoluteFilePath().startsWith(downloadDir);
            }
            match = true;

            qDebug() << "[Blocked Mods Dialog] Hash match found:" << mod.name << hash << "| From path:" << path;

            break;
        }
    }

    if (match) {
        update();
    }
}

bool BlockedModsDialog::checkValidPath(QString path)
{
    const QFileInfo file = QFileInfo(path);
    const QString filename = file.fileName();

    auto compare = [](QString fsFilename, QString metadataFilename) {
        return metadataFilename.compare(fsFilename, Qt::CaseInsensitive) == 0;
    };

    auto laxCompare = [](QString fsfilename, QString metadataFilename) {

        QList<QChar> allowedSeperators = { '-', '+', '.', '_' };

        auto fsName = fsfilename.toLower();
        auto metaName = metadataFilename.toLower();

        for (auto sep : allowedSeperators) {
            fsName = fsName.replace(sep, ' ');
            metaName = metaName.replace(sep, ' ');
        }

        fsName = fsName.simplified();
        metaName = metaName.simplified();

        return fsName.compare(metaName) == 0;
    };

    auto downloadDir = QFileInfo(APPLICATION->settings()->get("DownloadsDir").toString()).absoluteFilePath();
    auto moveFiles = APPLICATION->settings()->get("MoveModsFromDownloadsDir").toBool();
    for (auto& mod : m_mods) {
        if (compare(filename, mod.name)) {

            if (!mod.matched && mod.hash.isEmpty()) {
                mod.matched = true;
                mod.localPath = path;
                if (moveFiles) {
                    mod.move = QFileInfo(path).absoluteFilePath().startsWith(downloadDir);
                }
                return false;
            }
            qDebug() << "[Blocked Mods Dialog] Name match found:" << mod.name << "| From path:" << path;
            return true;
        }
        if (laxCompare(filename, mod.name)) {
            qDebug() << "[Blocked Mods Dialog] Lax name match found:" << mod.name << "| From path:" << path;
            return true;
        }
    }

    return false;
}

bool BlockedModsDialog::allModsMatched()
{
    return std::all_of(m_mods.begin(), m_mods.end(), [](auto const& mod) { return mod.matched; });
}

void BlockedModsDialog::validateMatchedMods()
{
    bool changed = false;
    for (auto& mod : m_mods) {
        if (mod.matched) {
            QFileInfo file = QFileInfo(mod.localPath);
            if (!file.exists() || !file.isFile()) {
                qDebug() << "[Blocked Mods Dialog] File" << mod.localPath << "for mod" << mod.name
                         << "has vanshed! marking as not matched.";
                mod.localPath = "";
                mod.matched = false;
                changed = true;
            }
        }
    }
    if (changed) {
        update();
    }
}

void BlockedModsDialog::runHashTask()
{
    if (!m_hashingTask->isRunning()) {
        m_rehashPending = false;

        if (!m_pendingHashPaths.isEmpty()) {
            qDebug() << "[Blocked Mods Dialog] there are pending hash tasks, building and running tasks";

            auto path = m_pendingHashPaths.begin();
            while (path != m_pendingHashPaths.end()) {
                buildHashTask(*path);
                path = m_pendingHashPaths.erase(path);
            }

            m_hashingTask->start();
        }
    } else {
        qDebug() << "[Blocked Mods Dialog] queueing another run of the hashing task";
        qDebug() << "[Blocked Mods Dialog] pending hash tasks:" << m_pendingHashPaths;
        m_rehashPending = true;
    }
}

void BlockedModsDialog::hashTaskFinished()
{
    qDebug() << "[Blocked Mods Dialog] All hash tasks finished";
    if (m_rehashPending) {
        qDebug() << "[Blocked Mods Dialog] task finished with a rehash pending, rerunning";
        runHashTask();
    }
}

QDebug operator<<(QDebug debug, const BlockedMod& m)
{
    QDebugStateSaver saver(debug);

    debug.nospace() << "{ name: " << m.name << ", websiteUrl: " << m.websiteUrl << ", hash: " << m.hash << ", matched: " << m.matched
                    << ", localPath: " << m.localPath << "}";

    return debug;
}
