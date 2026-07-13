// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (c) 2022 Jamie Mansfield <jmansfield@cadixdev.org>
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
 *  Copyright (C) 2023 TheKodeToad <TheKodeToad@proton.me>
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
#include <QList>
#include <QString>

class Config {
   public:
    Config();
    QString LAUNCHER_NAME;
    QString LAUNCHER_APP_BINARY_NAME;
    QString LAUNCHER_DISPLAYNAME;
    QString LAUNCHER_COPYRIGHT;
    QString LAUNCHER_DOMAIN;
    QString LAUNCHER_CONFIGFILE;
    QString LAUNCHER_GIT;
    QString LAUNCHER_APPID;
    QString LAUNCHER_SVGFILENAME;
    QString LAUNCHER_ENVNAME;

    int VERSION_MAJOR;

    int VERSION_MINOR;

    int VERSION_PATCH;

    QString VERSION_CHANNEL;

    bool UPDATER_ENABLED = false;
    bool JAVA_DOWNLOADER_ENABLED = false;

    QString BUILD_PLATFORM;

    QString BUILD_ARTIFACT;

    QString BUILD_DATE;

    QString COMPILER_NAME;

    QString COMPILER_VERSION;

    QString COMPILER_TARGET_SYSTEM;

    QString COMPILER_TARGET_SYSTEM_VERSION;

    QString COMPILER_TARGET_SYSTEM_PROCESSOR;

    QString UPDATER_GITHUB_REPO;

    QString MAC_SPARKLE_PUB_KEY;

    QString MAC_SPARKLE_APPCAST_URL;

    QString USER_AGENT;

    QString GIT_COMMIT;

    QString GIT_TAG;

    QString GIT_REFSPEC;

    QString NEWS_RSS_URL;

    QString NEWS_OPEN_URL;

    QString WIKI_URL;

    QString HELP_URL;

    QString LOGIN_CALLBACK_URL;

    QString IMGUR_CLIENT_ID;

    QString MSA_CLIENT_ID;

    QString ELY_CLIENT_ID;

    QString FLAME_API_KEY;

    QString META_URL;

    QString GLFW_LIBRARY_NAME;
    QString OPENAL_LIBRARY_NAME;

    QString BUG_TRACKER_URL;
    QString TRANSLATIONS_URL;
    QString MATRIX_URL;
    QString DISCORD_URL;
    QString SUBREDDIT_URL;

    QString DEFAULT_RESOURCE_BASE = "https://resources.download.minecraft.net/";
    QString LIBRARY_BASE = "https://libraries.minecraft.net/";
    QString IMGUR_BASE_URL = "https://api.imgur.com/3/";
    QString LEGACY_FMLLIBS_BASE_URL;
    QString TRANSLATION_FILES_URL;

    QString FTB_API_BASE_URL = "https://api.feed-the-beast.com/v1/modpacks/public";

    QString LEGACY_FTB_CDN_BASE_URL = "https://dist.creeper.host/FTB2/";

    QString ATL_DOWNLOAD_SERVER_URL = "https://download.nodecdn.net/containers/atl/";
    QString ATL_API_BASE_URL = "https://api.atlauncher.com/v1/";

    QString TECHNIC_API_BASE_URL = "https://api.technicpack.net/";

    QString TECHNIC_API_BUILD = "multimc";

    QString MODRINTH_STAGING_URL = "https://staging-api.modrinth.com/v2";
    QString MODRINTH_PROD_URL = "https://api.modrinth.com/v2";
    QStringList MODRINTH_MRPACK_HOSTS{ "cdn.modrinth.com", "github.com", "raw.githubusercontent.com", "gitlab.com" };
    QString MODRINTH_DOWNLOAD_HOST = "cdn.modrinth.com";

    QString FLAME_BASE_URL = "https://api.curseforge.com/v1";
    QString FLAME_DOWNLOAD_HOST = "edge.forgecdn.net";

    QString versionString() const;

    QString printableVersionString() const;

    QString compilerID() const;

    QString systemID() const;
};

extern const Config BuildConfig;
