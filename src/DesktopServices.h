#pragma once

#include <QString>
#include <QUrl>

class QFileInfo;

namespace DesktopServices
{

bool openPath(const QFileInfo& path, bool ensureFolderPathExists = false);

bool openPath(const QString& path, bool ensureFolderPathExists = false);

bool run(const QString& application, const QStringList& args, const QString& workingDirectory = QString(), qint64* pid = 0);

bool openUrl(const QUrl& url);

bool isFlatpak();

bool isSnap();
}  // namespace DesktopServices
