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

    virtual void checkForUpdates();
    void checkForUpdates(bool triggeredByUser) const;

    virtual bool getAutomaticallyChecksForUpdates();

    virtual double getUpdateCheckInterval();

    virtual bool getBetaAllowed();

    virtual void setAutomaticallyChecksForUpdates(bool check);

    virtual void setUpdateCheckInterval(double seconds);

    virtual void setBetaAllowed(bool allowed);

    void resetAutoCheckTimer() const;
    void disconnectTimer();
    void connectTimer();

    void offerUpdate(const QString& versionName, const QString& versionTag, const QString& releaseNotes, bool triggeredByUser) const;
    void performUpdate(const QString& versionTag) const;

   signals:
    void canCheckForUpdatesChanged(bool canCheck);

   public slots:
    void autoCheckTimerFired() const;

   private:
    class Private;

    Private* priv;
};
