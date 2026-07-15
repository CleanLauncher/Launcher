// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (c) 2022 flowln <flowlnlnln@gmail.com>
 *  Copyright (c) 2023 Trial97 <alexandru.tripon97@gmail.com>
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

#include "modplatform/ModIndex.h"

#include <QString>
#include <QUrl>
#include <QVariant>

class QDir;

namespace Packwiz {

class V1 {
   public:
    struct Mod {
        QString slug{};
        QString name{};
        QString filename{};
        ModPlatform::Side side{ ModPlatform::Side::UniversalSide };
        ModPlatform::ModLoaderTypes loaders;
        QStringList mcVersions;
        ModPlatform::IndexedVersionType releaseType;

        QString mode{};
        QUrl url{};
        QString hash_format{};
        QString hash{};

        ModPlatform::ResourceProvider provider{};
        QVariant file_id{};
        QVariant project_id{};
        QString version_number{};

        QList<ModPlatform::Dependency> dependencies;

       public:
        auto isValid() const -> bool { return !slug.isEmpty() && !project_id.isNull(); }

        auto mod_id() -> QVariant& { return project_id; }
        auto version() -> QVariant& { return file_id; }
    };

    static auto createModFormat(const QDir& index_dir, ModPlatform::IndexedPack& mod_pack, ModPlatform::IndexedVersion& mod_version) -> Mod;

    static void updateModIndex(const QDir& index_dir, Mod& mod);

    static void deleteModIndex(const QDir& index_dir, QString& mod_slug);

    static auto getIndexForMod(const QDir& index_dir, QString slug) -> Mod;

    static auto getIndexForMod(const QDir& index_dir, QVariant& mod_id) -> Mod;
};

}  // namespace Packwiz
