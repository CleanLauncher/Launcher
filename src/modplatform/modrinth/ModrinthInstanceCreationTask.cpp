#include "ModrinthInstanceCreationTask.h"

#include "Application.h"
#include "FileSystem.h"
#include "InstanceList.h"
#include "Json.h"

#include "QObjectPtr.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"

#include "minecraft/mod/Mod.h"
#include "modplatform/EnsureMetadataTask.h"
#include "modplatform/helpers/OverrideUtils.h"

#include "net/ChecksumValidator.h"

#include "net/ApiDownload.h"
#include "net/ApiHeaderProxy.h"
#include "net/NetJob.h"

#include "modplatform/ModIndex.h"
#include "settings/INISettingsObject.h"

#include "ui/dialogs/CustomMessageBox.h"
#include "ui/pages/modplatform/OptionalModDialog.h"

#include <QAbstractButton>
#include <QFileInfo>
#include <QHash>
#include <vector>

bool ModrinthCreationTask::abort()
{
    if (!canAbort()) {
        return false;
    }

    if (m_task) {
        m_task->abort();
    }
    return InstanceCreationTask::abort();
}

bool ModrinthCreationTask::updateInstance()
{
    auto* instanceList = APPLICATION->instances();

    BaseInstance* inst = nullptr;
    if (auto originalId = originalInstanceID(); !originalId.isEmpty()) {
        inst = instanceList->getInstanceById(originalId);
        Q_ASSERT(inst);
    } else {
        inst = instanceList->getInstanceByManagedName(originalName());

        if (!inst) {
            inst = instanceList->getInstanceById(originalName());

            if (!inst) {
                return false;
            }
        }
    }

    QString indexPath = FS::PathCombine(m_stagingPath, "modrinth.index.json");
    if (!parseManifest(indexPath, m_files, true, false)) {
        return false;
    }

    auto versionName = inst->getManagedPackVersionName();
    m_root_path      = QFileInfo(inst->gameRoot()).fileName();
    auto versionStr  = !versionName.isEmpty() ? tr(" (version %1)").arg(versionName) : "";

    if (shouldConfirmUpdate()) {
        auto shouldUpdate = askIfShouldUpdate(m_parent, versionStr);
        if (shouldUpdate == ShouldUpdate::SkipUpdating) {
            return false;
        }
        if (shouldUpdate == ShouldUpdate::Cancel) {
            m_abort = true;
            return false;
        }
    }

    QDir oldInstDir(inst->instanceRoot());

    QString oldIndexFolder(FS::PathCombine(oldInstDir.absolutePath(), "mrpack"));

    QString   oldIndexPath(FS::PathCombine(oldIndexFolder, "modrinth.index.json"));
    QFileInfo oldIndexFile(oldIndexPath);
    if (oldIndexFile.exists()) {
        std::vector<File> oldFiles;
        parseManifest(oldIndexPath, oldFiles, false, false);

        auto filesIterator = m_files.begin();
    begin:
        while (filesIterator != m_files.end()) {
            const auto& file = *filesIterator;

            auto oldFilesIterator = oldFiles.begin();
            while (oldFilesIterator != oldFiles.end()) {
                const auto& oldFile = *oldFilesIterator;

                if (oldFile.hash == file.hash) {
                    qDebug() << "Removed file at" << file.path << "from list of downloads";
                    filesIterator    = m_files.erase(filesIterator);
                    oldFilesIterator = oldFiles.erase(oldFilesIterator);
                    goto begin;
                }

                oldFilesIterator++;
            }

            filesIterator++;
        }

        QDir oldMinecraftDir(inst->gameRoot());

        if (!oldFiles.empty()) {
            for (const auto& file : oldFiles) {
                scheduleToDelete(m_parent, oldMinecraftDir, file.path, true);
            }
        }

        auto oldOverrides = Override::readOverrides("overrides", oldIndexFolder);
        for (const auto& entry : oldOverrides) {
            scheduleToDelete(m_parent, oldMinecraftDir, entry);
        }

        auto oldClientOverrides = Override::readOverrides("client-overrides", oldIndexFolder);
        for (const auto& entry : oldClientOverrides) {
            scheduleToDelete(m_parent, oldMinecraftDir, entry);
        }
    } else {
        auto* dialog = CustomMessageBox::selectable(m_parent,
                                                    tr("No index file."),
                                                    tr("We couldn't find a suitable index file for the older version. This may cause some "
                                                       "of the files to be duplicated. Do you want to continue?"),
                                                    QMessageBox::Warning,
                                                    QMessageBox::Ok | QMessageBox::Cancel);

        if (dialog->exec() == QDialog::DialogCode::Rejected) {
            m_abort = true;
            return false;
        }
    }

    setOverride(true, inst->id());
    qDebug() << "Will override instance!";

    m_instance = inst;

    return false;
}

std::unique_ptr<MinecraftInstance> ModrinthCreationTask::createInstance()
{
    QEventLoop loop;

    QString parentFolder(FS::PathCombine(m_stagingPath, "mrpack"));

    QString indexPath = FS::PathCombine(m_stagingPath, "modrinth.index.json");
    if (m_files.empty() && !parseManifest(indexPath, m_files, true, true)) {
        return nullptr;
    }

    QString newIndexPlace(FS::PathCombine(parentFolder, "modrinth.index.json"));
    FS::ensureFilePathExists(newIndexPlace);
    FS::move(indexPath, newIndexPlace);

    auto mcPath = FS::PathCombine(m_stagingPath, m_root_path);

    auto overridePath = FS::PathCombine(m_stagingPath, "overrides");
    if (QFile::exists(overridePath)) {
        Override::createOverrides("overrides", parentFolder, overridePath);

        if (!FS::move(overridePath, mcPath)) {
            setError(tr("Could not rename the overrides folder:\n") + "overrides");
            return nullptr;
        }
    }

    auto clientOverridePath = FS::PathCombine(m_stagingPath, "client-overrides");
    if (QFile::exists(clientOverridePath)) {
        Override::createOverrides("client-overrides", parentFolder, clientOverridePath);

        if (!FS::overrideFolder(mcPath, clientOverridePath)) {
            setError(tr("Could not rename the client overrides folder:\n") + "client overrides");
            return nullptr;
        }
    }

    QString configPath       = FS::PathCombine(m_stagingPath, "instance.cfg");
    auto    instanceSettings = std::make_unique<INISettingsObject>(configPath);
    auto    instance         = std::make_unique<MinecraftInstance>(m_globalSettings, std::move(instanceSettings), m_stagingPath);

    auto* components = instance->getPackProfile();
    components->buildingFromScratch();
    components->setComponentVersion("net.minecraft", m_minecraft_version, true);

    QString loader;
    if (!m_fabric_version.isEmpty()) {
        components->setComponentVersion("net.fabricmc.fabric-loader", m_fabric_version);
        loader = ModPlatform::getModLoaderAsString(ModPlatform::ModLoaderType::Fabric);
    }
    if (!m_quilt_version.isEmpty()) {
        components->setComponentVersion("org.quiltmc.quilt-loader", m_quilt_version);
        loader = ModPlatform::getModLoaderAsString(ModPlatform::ModLoaderType::Quilt);
    }
    if (!m_forge_version.isEmpty()) {
        components->setComponentVersion("net.minecraftforge", m_forge_version);
        loader = ModPlatform::getModLoaderAsString(ModPlatform::ModLoaderType::Forge);
    }
    if (!m_neoForge_version.isEmpty()) {
        components->setComponentVersion("net.neoforged", m_neoForge_version);
        loader = ModPlatform::getModLoaderAsString(ModPlatform::ModLoaderType::NeoForge);
    }

    if (m_instIcon != "default") {
        instance->setIconKey(m_instIcon);
    } else if (!m_managed_id.isEmpty()) {
        instance->setIconKey("modrinth");
    }

    if (!m_managed_id.isEmpty()) {
        instance->setManagedPack("modrinth", m_managed_id, m_managed_name, m_managed_version_id, version());
    } else {
        instance->setManagedPack("modrinth", "", name(), "", "");
    }

    instance->setName(name());
    instance->saveNow();

    auto downloadMods = makeShared<NetJob>(tr("Mod Download Modrinth"), APPLICATION->network());

    auto rootModpackPath = FS::PathCombine(m_stagingPath, m_root_path);
    auto rootModpackUrl  = QUrl::fromLocalFile(rootModpackPath);

    QHash<QString, Resource*> resources;
    for (auto& file : m_files) {
        auto fileName = file.path;
        fileName      = FS::RemoveInvalidPathChars(fileName);
        auto filePath = FS::PathCombine(rootModpackPath, fileName);
        if (!rootModpackUrl.isParentOf(QUrl::fromLocalFile(filePath))) {
            setError(tr("One of the files has a path that leads to an arbitrary location (%1). This is a security risk and isn't allowed.")
                         .arg(fileName));
            return nullptr;
        }
        if (fileName.startsWith("mods/")) {
            auto*      mod = new Mod(filePath);
            ModDetails d;
            d.mod_id = filePath;
            mod->setDetails(d);
            resources[file.hash.toHex()] = mod;
        }
        if (file.downloads.empty()) {
            setError(tr("The file '%1' is missing a download link. This is invalid in the pack format.").arg(fileName));
            return nullptr;
        }
        qDebug() << "Will try to download" << file.downloads.front() << "to" << filePath;

        Net::ModrinthDownloadMeta meta{
            .reason      = m_instance.has_value() ? "update" : "modpack",
            .gameVersion = m_minecraft_version,
            .loader      = loader,
        };

        QUrl downloadUrl = file.downloads.dequeue();
        auto dl          = Net::ApiDownload::makeFile(downloadUrl, filePath, Net::Download::Option::NoOptions, meta);
        dl->addValidator(new Net::ChecksumValidator(file.hashAlgorithm, file.hash));
        downloadMods->addNetAction(dl);
        if (!file.downloads.empty()) {
            auto param = dl.toWeakRef();
            connect(dl.get(), &Task::failed, [&file, filePath, param, downloadMods, meta] {
                QUrl fallbackUrl = file.downloads.dequeue();
                auto ndl         = Net::ApiDownload::makeFile(fallbackUrl, filePath, Net::Download::Option::NoOptions, meta);
                ndl->addValidator(new Net::ChecksumValidator(file.hashAlgorithm, file.hash));
                downloadMods->addNetAction(ndl);
                if (auto shared = param.lock()) {
                    shared->succeeded();
                }
            });
        }
    }

    bool endedWell = false;

    connect(downloadMods.get(), &NetJob::succeeded, this, [&endedWell]() { endedWell = true; });
    connect(downloadMods.get(), &NetJob::failed, [this, &endedWell](const QString& reason) {
        endedWell = false;
        setError(reason);
    });
    connect(downloadMods.get(), &NetJob::finished, &loop, &QEventLoop::quit);
    connect(downloadMods.get(), &NetJob::progress, [this](qint64 current, qint64 total) {
        setDetails(tr("%1 out of %2 complete").arg(current).arg(total));
        setProgress(current, total);
    });
    connect(downloadMods.get(), &NetJob::stepProgress, this, &ModrinthCreationTask::propagateStepProgress);

    setStatus(tr("Downloading mods..."));
    downloadMods->start();
    m_task = downloadMods;

    loop.exec();

    if (!endedWell) {
        for (auto* resource : resources) {
            delete resource;
        }
        return nullptr;
    }

    QEventLoop ensureMetaLoop;
    QDir       folder             = FS::PathCombine(instance->modsRoot(), ".index");
    auto       ensureMetadataTask = makeShared<EnsureMetadataTask>(resources, folder, ModPlatform::ResourceProvider::MODRINTH);
    connect(ensureMetadataTask.get(), &Task::succeeded, this, [&endedWell]() { endedWell = true; });
    connect(ensureMetadataTask.get(), &Task::finished, &ensureMetaLoop, &QEventLoop::quit);
    connect(ensureMetadataTask.get(), &Task::progress, [this](qint64 current, qint64 total) {
        setDetails(tr("%1 out of %2 complete").arg(current).arg(total));
        setProgress(current, total);
    });
    connect(ensureMetadataTask.get(), &Task::stepProgress, this, &ModrinthCreationTask::propagateStepProgress);

    ensureMetadataTask->start();
    m_task = ensureMetadataTask;

    ensureMetaLoop.exec();
    for (auto* resource : resources) {
        delete resource;
    }
    resources.clear();

    if (m_instance && endedWell) {
        setAbortable(false);
        auto* inst = m_instance.value();

        if (inst->name().contains(inst->getManagedPackVersionName()) && inst->name() != instance->name()) {
            if (askForChangingInstanceName(m_parent, inst->name(), instance->name()) == InstanceNameChange::ShouldChange) {
                inst->setName(instance->name());
            }
        }

        inst->copyManagedPack(*instance);
    }

    if (endedWell) {
        return instance;
    }
    return nullptr;
}

bool ModrinthCreationTask::parseManifest(const QString& indexPath, std::vector<File>& files, bool setInternalData, bool showOptionalDialog)
{
    try {
        auto doc           = Json::requireDocument(indexPath);
        auto obj           = Json::requireObject(doc, "modrinth.index.json");
        int  formatVersion = Json::requireInteger(obj, "formatVersion", "modrinth.index.json");
        if (formatVersion == 1) {
            auto game = Json::requireString(obj, "game", "modrinth.index.json");
            if (game != "minecraft") {
                throw JSONValidationError("Unknown game: " + game);
            }

            if (setInternalData) {
                if (m_managed_version_id.isEmpty()) {
                    m_managed_version_id = obj["versionId"].toString();
                }
                m_managed_name = obj["name"].toString();
            }

            auto              jsonFiles = Json::requireIsArrayOf<QJsonObject>(obj, "files", "modrinth.index.json");
            std::vector<File> optionalFiles;
            for (const auto& modInfo : jsonFiles) {
                File file;
                file.path = Json::requireString(modInfo, "path").replace("\\", "/");

                auto env = modInfo["env"].toObject();

                if (!env.isEmpty()) {
                    QString support = env["client"].toString("unsupported");
                    if (support == "unsupported") {
                        continue;
                    }
                    if (support == "optional") {
                        file.required = false;
                    }
                }

                QJsonObject hashes = Json::requireObject(modInfo, "hashes");
                file.hash          = QByteArray::fromHex(Json::requireString(hashes, "sha512").toLatin1());
                file.hashAlgorithm = QCryptographicHash::Sha512;

                auto downloadArr = modInfo["downloads"].toArray();
                for (auto download : downloadArr) {
                    qWarning() << download.toString();
                    bool isLast = download.toString() == downloadArr.last().toString();

                    auto downloadUrl = QUrl(download.toString());

                    if (!downloadUrl.isValid()) {
                        qDebug()
                            << QString("Download URL (%1) for %2 is not a correctly formatted URL").arg(downloadUrl.toString(), file.path);
                        if (isLast && file.downloads.isEmpty()) {
                            throw JSONValidationError(tr("Download URL for %1 is not a correctly formatted URL").arg(file.path));
                        }
                    } else {
                        file.downloads.push_back(downloadUrl);
                    }
                }

                (file.required ? files : optionalFiles).push_back(file);
            }

            if (!optionalFiles.empty()) {
                if (showOptionalDialog) {
                    QStringList oFiles;
                    for (const auto& file : optionalFiles) {
                        oFiles.push_back(file.path);
                    }
                    OptionalModDialog optionalModDialog(m_parent, oFiles);
                    if (optionalModDialog.exec() == QDialog::Rejected) {
                        emitAborted();
                        return false;
                    }

                    auto selectedMods = optionalModDialog.getResult();
                    for (auto file : optionalFiles) {
                        if (selectedMods.contains(file.path)) {
                            file.required = true;
                        } else {
                            file.path += ".disabled";
                        }
                        files.push_back(file);
                    }
                } else {
                    for (auto file : optionalFiles) {
                        file.path += ".disabled";
                        files.push_back(file);
                    }
                }
            }
            if (setInternalData) {
                auto dependencies = Json::requireObject(obj, "dependencies", "modrinth.index.json");
                for (auto it = dependencies.begin(), end = dependencies.end(); it != end; ++it) {
                    QString name = it.key();
                    if (name == "minecraft") {
                        m_minecraft_version = Json::requireString(*it, "Minecraft version");
                    } else if (name == "fabric-loader") {
                        m_fabric_version = Json::requireString(*it, "Fabric Loader version");
                    } else if (name == "quilt-loader") {
                        m_quilt_version = Json::requireString(*it, "Quilt Loader version");
                    } else if (name == "forge") {
                        m_forge_version = Json::requireString(*it, "Forge version");
                    } else if (name == "neoforge") {
                        m_neoForge_version = Json::requireString(*it, "NeoForge version");
                    } else {
                        throw JSONValidationError("Unknown dependency type: " + name);
                    }
                }
            }
        } else {
            throw JSONValidationError(QStringLiteral("Unknown format version: %s").arg(formatVersion));
        }

    } catch (const JSONValidationError& e) {
        setError(tr("Could not understand pack index:\n") + e.cause());
        return false;
    }

    return true;
}
