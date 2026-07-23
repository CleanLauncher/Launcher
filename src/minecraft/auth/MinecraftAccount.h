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

#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QPixmap>
#include <QString>

#include "AccountData.h"
#include "AuthSession.h"
#include "QObjectPtr.h"
#include "Usable.h"
#include "minecraft/auth/AuthFlow.h"

class Task;
class MinecraftAccount;

using MinecraftAccountPtr = shared_qobject_ptr<MinecraftAccount>;
Q_DECLARE_METATYPE(MinecraftAccountPtr)

struct AccountProfile {
    QString id;
    QString name;
    bool legacy;
};

class MinecraftAccount : public QObject, public Usable {
    Q_OBJECT
   public:
    explicit MinecraftAccount(const MinecraftAccount& other, QObject* parent) = delete;

    explicit MinecraftAccount(QObject* parent = 0);

    static MinecraftAccountPtr createBlank(AccountType type);

    static MinecraftAccountPtr createOffline(const QString& username);

    static MinecraftAccountPtr loadFromJsonV3(const QJsonObject& json);

    static QUuid uuidFromUsername(QString username);

    QJsonObject saveToJson() const;

   public:
    shared_qobject_ptr<AuthFlow> login(bool useDeviceCode = false);

    shared_qobject_ptr<AuthFlow> refresh();

    shared_qobject_ptr<AuthFlow> currentTask();

   public:
    QString internalId() const { return data.internalId; }

    QString accessToken() const { return data.accessToken(); }

    QString profileId() const { return data.profileId(); }

    QString profileName() const { return data.profileName(); }

    QString displayName() const;

    bool isActive() const;

    AccountType accountType() const noexcept { return data.type; }

    bool ownsMinecraft() const { return data.type != AccountType::MSA || data.minecraftEntitlement.ownsMinecraft; }

    bool hasProfile() const { return data.profileId().size() != 0; }

    QString typeString() const
    {
        switch (data.type) {
            case AccountType::MSA: {
                return "msa";
            } break;
            case AccountType::Ely: {
                return "msa";

            } break;
            case AccountType::Offline: {
                return "offline";
            } break;
            default: {
                return "unknown";
            }
        }
    }

    QString nameWithType() const
    {
        switch (data.type) {
            case AccountType::MSA: {
                return QString("%1 [MSA]").arg(profileName());
            } break;
            case AccountType::Ely: {
                return QString("%1 [Ely]").arg(profileName());
            } break;
            case AccountType::Offline: {
                return QString("%1 [Offline]").arg(profileName());
            } break;
            default: {
                return profileName();
            }
        }
    }

    QPixmap getFace(int width = 64, int height = 64) const;

    AccountState accountState() const;

    AccountData* accountData() { return &data; }

    bool shouldRefresh() const;

    void fillSession(AuthSessionPtr session, int elyPatchPreference);

    QString lastError() const { return data.lastError(); }

   signals:

    void changed();

    void activityChanged(bool active);

   protected:
    AccountData data;

    shared_qobject_ptr<AuthFlow> m_currentTask;

   protected:
    void incrementUses() override;
    void decrementUses() override;

   private slots:
    void authSucceeded();
    void authFailed(QString reason);
};
