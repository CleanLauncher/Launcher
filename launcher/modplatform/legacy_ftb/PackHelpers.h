#pragma once

#include <QList>
#include <QMetaType>
#include <QString>
#include <QStringList>

namespace LegacyFTB {

enum class PackType { Public, ThirdParty, Private };

struct Modpack {
    QString name;
    QString description;
    QString author;
    QStringList oldVersions;
    QString currentVersion;
    QString mcVersion;
    QString mods;
    QString logo;

    QString dir;
    QString file;

    bool bugged = false;
    bool broken = false;

    PackType type;
    QString packCode;
};

using ModpackList = QList<Modpack>;

}

Q_DECLARE_METATYPE(LegacyFTB::Modpack)
