// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher: Minecraft Launcher
 *  Copyright (c) 2023-2024 Trial97 <alexandru.tripon97@gmail.com>
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
#pragma once

#include "Application.h"
#include <QFileInfo>
#include <QSaveFile>

#if defined(LAUNCHER_APPLICATION)

class PSaveFile : public QSaveFile
{
public:
    PSaveFile(const QString& name) : QSaveFile(name) { addPath(name); }
    PSaveFile(const QString& name, QObject* parent) : QSaveFile(name, parent) { addPath(name); }
    virtual ~PSaveFile()
    {
        if (auto app = APPLICATION_DYN) {
            app->removeQSavePath(m_absoluteFilePath);
        }
    }

private:
    void addPath(const QString& path)
    {
        m_absoluteFilePath = QFileInfo(path).absoluteFilePath() + ".";

        if (auto app = APPLICATION_DYN) {
            app->addQSavePath(m_absoluteFilePath);
        }
    }
    QString m_absoluteFilePath;
};
#else
using PSaveFile = QSaveFile;
#endif
