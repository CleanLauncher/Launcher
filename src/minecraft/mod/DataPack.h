// SPDX-FileCopyrightText: 2022 Rachel Powers <508861+Ryex@users.noreply.github.com>

// SPDX-License-Identifier: GPL-3.0-only

/*
 *  Prism Launcher: Minecraft Launcher
 *  Copyright (C) 2022 Rachel Powers <508861+Ryex@users.noreply.github.com>
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

#pragma once

#include "Resource.h"

#include <QMutex>
#include <QPixmapCache>
#include <utility>

class Version;

class DataPack : public Resource
{
    Q_OBJECT
public:
    DataPack(QObject* parent = nullptr) : Resource(parent) {}
    DataPack(QFileInfo file_info) : Resource(file_info) {}

    int packFormat() const { return m_pack_format; }

    QString description() const { return m_description; }

    QPixmap image(QSize size, Qt::AspectRatioMode mode = Qt::AspectRatioMode::IgnoreAspectRatio) const;

    void setPackFormat(int new_format_id, std::pair<int, int> min_format, std::pair<int, int> max_format);

    void setDescription(QString new_description);

    void setImage(QImage new_image) const;

    bool valid() const override;

    [[nodiscard]] int  compare(const Resource& other, SortType type) const override;
    [[nodiscard]] bool applyFilter(QRegularExpression filter) const override;

    QString packFormatStr() const;

protected:
    virtual QMap<std::pair<int, int>, std::pair<Version, Version>> mappings() const;

protected:
    mutable QMutex m_data_lock;

    int                 m_pack_format = 0;
    std::pair<int, int> m_min_format;
    std::pair<int, int> m_max_format;

    QString m_description;

    struct
    {
        QPixmapCache::Key key;
        bool              was_ever_used = false;
    } mutable m_pack_image_cache_key;
};
