// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (c) 2023 Trial97 <alexandru.tripon97@gmail.com>
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

#include <QDateTime>
#include <QFileInfo>
#include <QObject>
#include <QPointer>

#include "MetadataHandler.h"
#include "QObjectPtr.h"

class BaseInstance;

enum class ResourceType : std::uint8_t {
    UNKNOWN,

    ZIPFILE,

    SINGLEFILE,

    FOLDER,

    LITEMOD,

};

QDebug operator<<(QDebug debug, ResourceType type);

enum class ResourceStatus : std::uint8_t {
    Installed,

    NotInstalled,

    NoMetadata,

    Unknown,

};

QDebug operator<<(QDebug debug, ResourceStatus status);

enum class SortType : std::uint8_t {
    Name,
    Date,
    Version,
    Enabled,
    PackFormat,
    Provider,
    Size,
    Side,
    McVersions,
    Loaders,
    ReleaseType,
    Requires,
    RequiredBy,
    Filename,
};

enum class EnableAction : std::uint8_t { ENABLE, DISABLE, TOGGLE };

class Resource : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Resource)
   public:
    using Ptr = shared_qobject_ptr<Resource>;

    Resource(QObject* parent = nullptr);
    Resource(QFileInfo fileInfo);
    Resource(const QString& filePath) : Resource(QFileInfo(filePath)) {}

    ~Resource() override = default;

    void setFile(QFileInfo fileInfo);
    void parseFile();

    auto fileinfo() const -> QFileInfo { return m_file_info; }
    auto dateTimeChanged() const -> QDateTime { return m_changed_date_time; }
    auto internalId() const -> QString { return m_internal_id; }
    auto type() const -> ResourceType { return m_type; }
    bool enabled() const { return m_enabled; }
    auto getOriginalFileName() const -> QString;
    QString sizeStr() const { return m_size_str; }
    qint64 sizeInfo() const { return m_size_info; }

    virtual auto name() const -> QString;
    virtual bool valid() const { return m_type != ResourceType::UNKNOWN; }

    auto status() const -> ResourceStatus { return m_status; };
    auto metadata() -> std::shared_ptr<Metadata::ModStruct> { return m_metadata; }
    auto metadata() const -> std::shared_ptr<const Metadata::ModStruct> { return m_metadata; }
    auto provider() const -> QString;
    virtual auto homepage() const -> QString;

    void setStatus(ResourceStatus status) { m_status = status; }
    void setMetadata(std::shared_ptr<Metadata::ModStruct>&& metadata);
    void setMetadata(const Metadata::ModStruct& metadata) { setMetadata(std::make_shared<Metadata::ModStruct>(metadata)); }

    QStringList issues() const;
    void updateIssues(const BaseInstance* inst);
    bool hasIssues() const { return !m_issues.empty(); }

    virtual int compare(const Resource& other, SortType type = SortType::Name) const;

    virtual bool applyFilter(QRegularExpression filter) const;

    bool enable(EnableAction action);

    auto shouldResolve() const -> bool { return !m_is_resolving && !m_is_resolved; }
    auto isResolving() const -> bool { return m_is_resolving; }
    auto isResolved() const -> bool { return m_is_resolved; }
    auto resolutionTicket() const -> int { return m_resolution_ticket; }

    void setResolving(bool resolving, int resolutionTicket)
    {
        m_is_resolving = resolving;
        m_resolution_ticket = resolutionTicket;
    }

    auto destroy(const QDir& indexDir, bool preserveMetadata = false, bool attemptTrash = true) -> bool;

    auto destroyMetadata(const QDir& indexDir) -> void;

    auto isSymLink() const -> bool { return m_file_info.isSymLink(); }

    bool isSymLinkUnder(const QString& instPath) const;

    bool isMoreThanOneHardLink() const;

   protected:
    QFileInfo m_file_info;

    QDateTime m_changed_date_time;

    QString m_internal_id;

    QString m_name;

    ResourceType m_type = ResourceType::UNKNOWN;

    ResourceStatus m_status = ResourceStatus::Unknown;

    std::shared_ptr<Metadata::ModStruct> m_metadata = nullptr;

    bool m_enabled = true;

    QList<const char*> m_issues;

    bool m_is_resolving = false;
    bool m_is_resolved = false;
    int m_resolution_ticket = 0;
    QString m_size_str;
    qint64 m_size_info;
};
