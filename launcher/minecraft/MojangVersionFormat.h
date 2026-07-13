#pragma once

#include <ProblemProvider.h>
#include <minecraft/Library.h>
#include <minecraft/VersionFile.h>
#include <QJsonDocument>

class MojangVersionFormat {
    friend class OneSixVersionFormat;

   protected:

    static void readVersionProperties(const QJsonObject& in, VersionFile* out);

    static void writeVersionProperties(const VersionFile* in, QJsonObject& out);

   public:

    static VersionFilePtr versionFileFromJson(const QJsonDocument& doc, const QString& filename);
    static QJsonDocument versionFileToJson(const VersionFilePtr& patch);

    static LibraryPtr libraryFromJson(ProblemContainer& problems, const QJsonObject& libObj, const QString& filename);
    static QJsonObject libraryToJson(Library* library);
};
