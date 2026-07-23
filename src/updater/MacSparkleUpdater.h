// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher: Minecraft Launcher
 *  Copyright (C) 2022 Kenneth Chew <kenneth.c0@protonmail.com>
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

#ifndef LAUNCHER_MACSPARKLEUPDATER_H
#define LAUNCHER_MACSPARKLEUPDATER_H

#include "ExternalUpdater.h"
#include <QObject>
#include <QSet>

class MacSparkleUpdater : public ExternalUpdater
{
    Q_OBJECT

public:
    MacSparkleUpdater();
    ~MacSparkleUpdater() override;

    void checkForUpdates() override;

    bool getAutomaticallyChecksForUpdates() override;

    double getUpdateCheckInterval() override;

    QSet<QString> getAllowedChannels();

    bool getBetaAllowed() override;

    void setAutomaticallyChecksForUpdates(bool check) override;

    void setUpdateCheckInterval(double seconds) override;

    void clearAllowedChannels();

    void setAllowedChannel(const QString& channel);

    void setAllowedChannels(const QSet<QString>& channels);

    void setBetaAllowed(bool allowed) override;

private:
    class Private;

    Private* priv;
};

#endif
