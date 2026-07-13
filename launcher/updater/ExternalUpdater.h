// SPDX-FileCopyrightText: 2023 Rachel Powers <508861+Ryex@users.noreply.github.com>

// SPDX-License-Identifier: GPL-3.0-only

/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2023 Rachel Powers <508861+Ryex@users.noreply.github.com>
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
 */

#pragma once

class ExternalUpdater : public QObject {
    Q_OBJECT

   public:
    ExternalUpdater(QWidget* parent, const QString& appDir, const QString& dataDir);
    ~ExternalUpdater() override;

    void checkForUpdates() override;
    void checkForUpdates(bool triggeredByUser) const;

    bool getAutomaticallyChecksForUpdates() override;

    double getUpdateCheckInterval() override;

    bool getBetaAllowed() override;

    void setAutomaticallyChecksForUpdates(bool check) override;

    void setUpdateCheckInterval(double seconds) override;

    void setBetaAllowed(bool allowed) override;

    void resetAutoCheckTimer() const;
    void disconnectTimer();
    void connectTimer();

    void offerUpdate(const QString& versionName, const QString& versionTag, const QString& releaseNotes, bool triggeredByUser) const;
    void performUpdate(const QString& versionTag) const;

   public slots:
    void autoCheckTimerFired() const;

   private:
    class Private;

    Private* priv;
};
