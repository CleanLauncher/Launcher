#pragma once

#include <ProblemProvider.h>
#include <QJsonDocument>
#include <minecraft/Library.h>
#include <minecraft/PackProfile.h>
#include <minecraft/VersionFile.h>

class OneSixVersionFormat
{
public:
    static VersionFilePtr versionFileFromJson(const QJsonDocument& doc, const QString& filename, bool requireOrder);
    static QJsonDocument  versionFileToJson(const VersionFilePtr& patch);

    static LibraryPtr  libraryFromJson(ProblemContainer& problems, const QJsonObject& libObj, const QString& filename);
    static QJsonObject libraryToJson(Library* library);

    static LibraryPtr
    plusJarModFromJson(ProblemContainer& problems, const QJsonObject& libObj, const QString& filename, const QString& originalName);

    static LibraryPtr  jarModFromJson(ProblemContainer& problems, const QJsonObject& libObj, const QString& filename);
    static QJsonObject jarModtoJson(Library* jarmod);

    static LibraryPtr  modFromJson(ProblemContainer& problems, const QJsonObject& libObj, const QString& filename);
    static QJsonObject modtoJson(Library* jarmod);
};
