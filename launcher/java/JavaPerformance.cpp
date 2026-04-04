// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2026 Octol1ttle <l1ttleofficial@outlook.com>
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

#include "JavaPerformance.h"

#include <QDebug>
#include <QObject>

QStringList JavaPerformance::getMojangOptimizationArgs(const JavaVersion& version, const GarbageCollectorPreset preset)
{
    QStringList args;

    // -XX:+UseCompactObjectHeaders
    // JEPs: https://openjdk.org/jeps/450, https://openjdk.org/jeps/519
    if (version.major() >= 24) {
        if (version.major() == 24) {
            args << "-XX:+UnlockExperimentalVMOptions";
        }
        args << "-XX:+UseCompactObjectHeaders";
    }

    // -XX:+AlwaysPreTouch
    // Since Java 6 (minimum supported by us is Java 7)
    args << "-XX:+AlwaysPreTouch";

    // -XX:+UseStringDeduplication
    // Since Java 8 for G1GC
    // Since Java 18 for ZGC
    if ((version.major() >= 8 && preset == GarbageCollectorPreset::G1GC) || version.major() >= 18) {
        args << "-XX:+UseStringDeduplication";
    }

    return args;
}

QStringList JavaPerformance::getGarbageCollectorArgs(const JavaVersion& version, const GarbageCollectorPreset preset)
{
    switch (preset) {
        case GarbageCollectorPreset::None:
            return {};
        case GarbageCollectorPreset::G1GC:
            return { "-XX:+UnlockExperimentalVMOptions", "-XX:+UseG1GC",
                     "-XX:G1NewSizePercent=20",          "-XX:G1ReservePercent=20",
                     "-XX:MaxGCPauseMillis=50",          "-XX:G1HeapRegionSize=32M" };
        case GarbageCollectorPreset::ZGC:
            QStringList args{ "-XX:+UseZGC" };
            // Support for generations was added in Java 21 and became default in Java 23
            if (version.major() >= 21 && version.major() < 23) {
                args << "-XX:+ZGenerational";
            }
            return args;
    }

    Q_ASSERT_X(false, "JavaPerformance::getGarbageCollectorArgs", "No arguments specified for current garbage collector preset");
    return {};
}

QStringList JavaPerformance::getCompletePerformanceArgs(const JavaVersion& version, const bool useOptimized, GarbageCollectorPreset preset, QString* warning)
{
    // ZGC was declared as production ready in Java 15, but did not receive support for macOS/aarch64 until Java 17
    if (preset == GarbageCollectorPreset::ZGC && version.major() < 17) {
        preset = GarbageCollectorPreset::G1GC;
        if (warning) {
            *warning = QObject::tr("ZGC requires Java 17 or higher, using G1GC");
        }
    }

    if (!useOptimized) {
        return getGarbageCollectorArgs(version, preset);
    }
    return getMojangOptimizationArgs(version, preset) + getGarbageCollectorArgs(version, preset);
}
