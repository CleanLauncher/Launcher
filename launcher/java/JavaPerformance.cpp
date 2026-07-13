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

#include <QObject>

QStringList JavaPerformance::getBaseOptimizationArgs(const JavaVersion& version, const GarbageCollectorPreset preset)
{
    QStringList args;

    if (version.major() >= 24) {
        if (version.major() == 24) {
            args << "-XX:+UnlockExperimentalVMOptions";
        }
        args << "-XX:+UseCompactObjectHeaders";
    }

    if ((version.major() >= 8 && preset != GarbageCollectorPreset::ZGC) || version.major() >= 18) {
        args << "-XX:+UseStringDeduplication";
    }

    if (version.major() <= 25) {
        args << "-XX:+ParallelRefProcEnabled";
    }

    args << "-XX:+AlwaysPreTouch";

    args << "-XX:+PerfDisableSharedMem";

    return args;
}

QStringList JavaPerformance::getGarbageCollectorArgs(const JavaVersion& version, const GarbageCollectorPreset preset)
{
    switch (preset) {
        case GarbageCollectorPreset::None:
            return {};
        case GarbageCollectorPreset::G1GC: {
            QStringList args{ "-XX:+UnlockExperimentalVMOptions",
                              "-XX:+UseG1GC",
                              "-XX:G1NewSizePercent=20",
                              "-XX:G1ReservePercent=20",
                              "-XX:MaxGCPauseMillis=50",
                              "-XX:G1HeapRegionSize=32M",

                              "-XX:SurvivorRatio=32",
                              "-XX:MaxTenuringThreshold=1" };
            return args;
        }
        case GarbageCollectorPreset::Shenandoah: {
            QStringList args{ "-XX:+UseShenandoahGC" };
            if (version.major() >= 24) {
                if (version.major() == 24) {
                    args << "-XX:+UnlockExperimentalVMOptions";
                }
                args << "-XX:ShenandoahGCMode=generational";
            }
            return args;
        }
        case GarbageCollectorPreset::ZGC: {
            QStringList args{ "-XX:+UseZGC" };

            if (version.major() >= 21 && version.major() < 23) {
                args << "-XX:+ZGenerational";
            }

            args << "-XX:-ZUncommit";
            return args;
        }
    }

    Q_ASSERT_X(false, "JavaPerformance::getGarbageCollectorArgs", "No arguments specified for current garbage collector preset");
    return {};
}

QStringList JavaPerformance::getCompletePerformanceArgs(const JavaVersion& version,
                                                        const bool useOptimized,
                                                        GarbageCollectorPreset preset,
                                                        QString* warning)
{
    GarbageCollectorPreset maximumSupported;
    if (version.major() >= 17) {
        maximumSupported = GarbageCollectorPreset::ZGC;
    } else if (version.major() >= 11) {
        maximumSupported = GarbageCollectorPreset::Shenandoah;
    } else {
        maximumSupported = GarbageCollectorPreset::G1GC;
    }

    if (preset > maximumSupported) {
        preset = maximumSupported;
        if (warning) {
            *warning = QObject::tr("%1 may be used instead because the Java version is too old").arg(presetToString(maximumSupported));
        }
    }

    if (!useOptimized) {
        return getGarbageCollectorArgs(version, preset);
    }

    auto allArgs = getBaseOptimizationArgs(version, preset) + getGarbageCollectorArgs(version, preset);
    allArgs.removeDuplicates();
    return allArgs;
}
