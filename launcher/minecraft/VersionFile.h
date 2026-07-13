// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2013-2021 MultiMC Contributors
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#pragma once

#include <QDateTime>
#include <QHash>
#include <QList>
#include <QSet>
#include <QString>
#include <QStringList>

#include <meta/JsonFormat.h>
#include <memory>
#include "Agent.h"
#include "Library.h"
#include "ProblemProvider.h"
#include "java/JavaMetadata.h"
#include "minecraft/Rule.h"

class PackProfile;
class VersionFile;
class LaunchProfile;
struct MojangDownloadInfo;
struct MojangAssetIndexInfo;

using VersionFilePtr = std::shared_ptr<VersionFile>;
class VersionFile : public ProblemContainer {
    friend class MojangVersionFormat;
    friend class OneSixVersionFormat;

   public:
    void applyTo(LaunchProfile* profile, const RuntimeContext& runtimeContext);

   public:

    int order = 0;

    QString name;

    QString uid;

    QString version;

    QString dependsOnMinecraftVersion;

    int minimumLauncherVersion = -1;

    QString minecraftVersion;

    QString mainClass;

    QString appletClass;

    QString minecraftArguments;

    QStringList addnJvmArguments;

    QList<int> compatibleJavaMajors;

    QString compatibleJavaName;

    QString type;

    QDateTime releaseTime;

    QDateTime updateTime;

    QString assets;

    QStringList addTweakers;

    QList<LibraryPtr> libraries;

    QList<LibraryPtr> mavenFiles;

    QList<Agent> agents;

    LibraryPtr mainJar;

    QSet<QString> traits;

    QList<LibraryPtr> jarMods;

    QList<LibraryPtr> mods;

    Meta::RequireSet m_requires;

    Meta::RequireSet conflicts;

    bool m_volatile = false;

    QList<Java::MetadataPtr> runtimes;

   public:

    QMap<QString, std::shared_ptr<MojangDownloadInfo>> mojangDownloads;

    std::shared_ptr<MojangAssetIndexInfo> mojangAssetIndex;
};
