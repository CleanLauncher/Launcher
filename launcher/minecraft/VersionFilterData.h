#pragma once
#include <QDateTime>
#include <QMap>
#include <QSet>
#include <QString>

struct FMLlib {
    QString filename;
    QString checksum;
};

struct VersionFilterData {
    VersionFilterData();

    QMap<QString, QList<FMLlib>> fmlLibsMapping;

    QSet<QString> forgeInstallerBlacklist;

    QDateTime legacyCutoffDate;

    QSet<QString> lwjglWhitelist;

    QDateTime java8BeginsDate;

    QDateTime java16BeginsDate;

    QDateTime java17BeginsDate;
};
extern VersionFilterData g_VersionFilterData;
