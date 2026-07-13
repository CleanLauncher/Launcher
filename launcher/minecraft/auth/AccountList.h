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

#include "MinecraftAccount.h"
#include "minecraft/auth/AuthFlow.h"

#include <QAbstractListModel>
#include <QObject>
#include <QSharedPointer>
#include <QVariant>

class AccountList : public QAbstractListModel {
    Q_OBJECT
   public:
    enum ModelRoles { PointerRole = 0x34B1CB48 };

    enum VListColumns {

        ProfileNameColumn = 0,
        TypeColumn,
        StatusColumn,

        NUM_COLUMNS
    };

    explicit AccountList(QObject* parent = 0);
    virtual ~AccountList() noexcept;

    const MinecraftAccountPtr at(int i) const;
    int count() const;

    QVariant data(const QModelIndex& index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual int rowCount(const QModelIndex& parent) const override;
    virtual int columnCount(const QModelIndex& parent) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    void addAccount(MinecraftAccountPtr account);
    void removeAccount(QModelIndex index);
    void moveAccount(QModelIndex index, int delta);
    int findAccountByProfileId(const QString& profileId) const;
    MinecraftAccountPtr getAccountByProfileName(const QString& profileName) const;
    QStringList profileNames() const;

    void requestRefresh(QString accountId);

    void queueRefresh(QString accountId);

    void setListFilePath(QString path, bool autosave = false);

    bool loadList();
    bool loadV3(QJsonObject& root);
    bool saveList();

    MinecraftAccountPtr defaultAccount() const;
    void setDefaultAccount(MinecraftAccountPtr profileId);
    bool anyAccountIsValid();
    bool isEmpty() const;

    bool isActive() const;

   protected:
    void beginActivity();
    void endActivity();

   private:
    uint32_t m_activityCount = 0;
   signals:
    void listChanged();
    void listActivityChanged();
    void defaultAccountChanged();
    void activityChanged(bool active);

   public slots:

    void accountChanged();

    void accountActivityChanged(bool active);

    void fillQueue();

   private slots:
    void tryNext();

    void authSucceeded();
    void authFailed(QString reason);

   protected:
    QList<QString> m_refreshQueue;
    QTimer* m_refreshTimer;
    QTimer* m_nextTimer;
    shared_qobject_ptr<AuthFlow> m_currentTask;

    void onListChanged();

    void onDefaultAccountChanged();

    QList<MinecraftAccountPtr> m_accounts;

    MinecraftAccountPtr m_defaultAccount;

    QString m_listFilePath;

    bool m_autosave = false;
};
