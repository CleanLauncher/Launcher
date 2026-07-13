#pragma once
#include <QMap>
#include <QString>
#include <memory>

struct MojangDownloadInfo {

    using Ptr = std::shared_ptr<MojangDownloadInfo>;

    QString path;

    QString url;

    QString sha1;

    int size;
};

struct MojangLibraryDownloadInfo {
    MojangLibraryDownloadInfo(MojangDownloadInfo::Ptr artifact_) : artifact(artifact_) {}
    MojangLibraryDownloadInfo() {}

    using Ptr = std::shared_ptr<MojangLibraryDownloadInfo>;

    MojangDownloadInfo* getDownloadInfo(QString classifier)
    {
        if (classifier.isNull()) {
            return artifact.get();
        }

        return classifiers[classifier].get();
    }

    MojangDownloadInfo::Ptr artifact;
    QMap<QString, MojangDownloadInfo::Ptr> classifiers;
};

struct MojangAssetIndexInfo : public MojangDownloadInfo {

    using Ptr = std::shared_ptr<MojangAssetIndexInfo>;

    MojangAssetIndexInfo() {}

    MojangAssetIndexInfo(QString id_)
    {
        this->id = id_;

        if (id_ == "legacy") {
            url = "https://piston-meta.mojang.com/mc/assets/legacy/c0fd82e8ce9fbc93119e40d96d5a4e62cfa3f729/legacy.json";
        }

        else {
            url = "https://s3.amazonaws.com/Minecraft.Download/indexes/" + id_ + ".json";
        }
        known = false;
    }

    int totalSize;
    QString id;
    bool known = true;
};
