// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2023 Tayou <git@tayou.org>
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

#include <QList>
#include <QRect>
#include <QString>
#include <QStyleOption>

class InstanceView;
class QPainter;
class QModelIndex;

struct VisualRow {
    QList<QModelIndex> items;
    int height = 0;
    int top = 0;
    inline int size() const { return items.size(); }
    inline QModelIndex& operator[](int i) { return items[i]; }
};

struct VisualGroup {
    VisualGroup(QString text, InstanceView* view);
    explicit VisualGroup(const VisualGroup* other);

    InstanceView* view = nullptr;
    QString text;
    bool collapsed = false;
    QList<VisualRow> rows;
    int firstItemIndex = 0;
    int m_verticalPosition = 0;

    void update();

    void drawHeader(QPainter* painter, const QStyleOptionViewItem& option) const;

    int totalHeight() const;

    static int headerHeight();

    int contentHeight() const;

    int numRows() const;

    int calculateNumRows() const;

    int verticalPosition() const;

    int rowTopOf(const QModelIndex& index) const;

    int rowHeightOf(const QModelIndex& index) const;

    QPair<int, int> positionOf(const QModelIndex& index) const;

    enum HitResult { NoHit = 0x0, TextHit = 0x1, CheckboxHit = 0x2, HeaderHit = 0x4, BodyHit = 0x8 };
    Q_DECLARE_FLAGS(HitResults, HitResult)

    HitResults hitScan(const QPoint& pos) const;

    QList<QModelIndex> items() const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(VisualGroup::HitResults)
