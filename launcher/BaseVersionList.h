/* Copyright 2013-2021 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QVariant>

#include "BaseVersion.h"
#include "QObjectPtr.h"
#include "tasks/Task.h"

class BaseVersionList : public QAbstractListModel {
    Q_OBJECT
   public:
    enum ModelRoles {
        VersionPointerRole = Qt::UserRole,
        VersionRole,
        VersionIdRole,
        ParentVersionRole,
        RecommendedRole,
        LatestRole,
        TypeRole,
        BranchRole,
        PathRole,
        JavaNameRole,
        JavaMajorRole,
        CPUArchitectureRole,
        SortRole
    };
    using RoleList = QList<int>;

    explicit BaseVersionList(QObject* parent = 0);

    virtual Task::Ptr getLoadTask(bool forceReload = false) = 0;

    virtual bool isLoaded() = 0;

    virtual const BaseVersion::Ptr at(int i) const = 0;

    virtual int count() const = 0;

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QHash<int, QByteArray> roleNames() const override;

    virtual RoleList providesRoles() const;

    virtual BaseVersion::Ptr findVersion(const QString& descriptor);

    virtual BaseVersion::Ptr getRecommended() const;

    virtual void sortVersions() = 0;

   protected slots:

    virtual void updateListData(QList<BaseVersion::Ptr> versions) = 0;
};
