// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher: Minecraft Launcher
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
#include <ProblemProvider.h>
#include <QString>
#include "Agent.h"
#include "Library.h"

class LaunchProfile : public ProblemProvider {
   public:
    virtual ~LaunchProfile() {}

   public:
    void applyMinecraftVersion(const QString& id);
    void applyMainClass(const QString& mainClass);
    void applyAppletClass(const QString& appletClass);
    void applyMinecraftArguments(const QString& minecraftArguments);
    void applyAddnJvmArguments(const QStringList& minecraftArguments);
    void applyMinecraftVersionType(const QString& type);
    void applyMinecraftAssets(MojangAssetIndexInfo::Ptr assets);
    void applyTraits(const QSet<QString>& traits);
    void applyTweakers(const QStringList& tweakers);
    void applyJarMods(const QList<LibraryPtr>& jarMods);
    void applyMods(const QList<LibraryPtr>& jarMods);
    void applyLibrary(LibraryPtr library, const RuntimeContext& runtimeContext);
    void applyMavenFile(LibraryPtr library, const RuntimeContext& runtimeContext);
    void applyAgent(const Agent& agent, const RuntimeContext& runtimeContext);
    void applyCompatibleJavaMajors(QList<int>& javaMajor);
    void applyCompatibleJavaName(QString javaName);
    void applyMainJar(LibraryPtr jar);
    void applyProblemSeverity(ProblemSeverity severity);

    void clear();

   public:
    QString getMinecraftVersion() const;
    QString getMainClass() const;
    QString getAppletClass() const;
    QString getMinecraftVersionType() const;
    MojangAssetIndexInfo::Ptr getMinecraftAssets() const;
    QString getMinecraftArguments() const;
    const QStringList& getAddnJvmArguments() const;
    const QSet<QString>& getTraits() const;
    const QStringList& getTweakers() const;
    const QList<LibraryPtr>& getJarMods() const;
    const QList<LibraryPtr>& getLibraries() const;
    QList<LibraryPtr>& getLibrariesMut() { return m_libraries; }
    const QList<LibraryPtr>& getNativeLibraries() const;
    const QList<LibraryPtr>& getMavenFiles() const;
    const QList<Agent>& getAgents() const;
    const QList<int>& getCompatibleJavaMajors() const;
    const QString getCompatibleJavaName() const;
    const LibraryPtr getMainJar() const;
    void getLibraryFiles(const RuntimeContext& runtimeContext,
                         QStringList& jars,
                         QStringList& nativeJars,
                         const QString& overridePath,
                         const QString& tempPath,
                         bool addJarMods = true) const;
    bool hasTrait(const QString& trait) const;
    ProblemSeverity getProblemSeverity() const override;
    const QList<PatchProblem> getProblems() const override;

   private:
    QString m_minecraftVersion;

    QString m_minecraftVersionType;

    MojangAssetIndexInfo::Ptr m_minecraftAssets;

    QString m_minecraftArguments;

    QStringList m_addnJvmArguments;

    QStringList m_tweakers;

    QString m_mainClass;

    QString m_appletClass;

    QList<LibraryPtr> m_libraries;

    QList<LibraryPtr> m_mavenFiles;

    QList<Agent> m_agents;

    LibraryPtr m_mainJar;

    QList<LibraryPtr> m_nativeLibraries;

    QSet<QString> m_traits;

    QList<LibraryPtr> m_jarMods;

    QList<LibraryPtr> m_mods;

    QList<int> m_compatibleJavaMajors;

    QString m_compatibleJavaName;

    ProblemSeverity m_problemSeverity = ProblemSeverity::None;
};
