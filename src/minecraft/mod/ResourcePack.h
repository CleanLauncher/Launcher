#pragma once

#include "Resource.h"
#include "minecraft/mod/DataPack.h"

#include <QImage>
#include <QMutex>
#include <QPixmap>
#include <QPixmapCache>

class Version;

class ResourcePack : public DataPack
{
    Q_OBJECT
public:
    ResourcePack(QObject* parent = nullptr) : DataPack(parent) {}
    ResourcePack(QFileInfo file_info) : DataPack(file_info) {}

    QMap<std::pair<int, int>, std::pair<Version, Version>> mappings() const override;
};
