// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (C) 2022 Lenny McLennington <lenny@sneed.church>
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

#include "FetchFlameAPIKey.h"

#include "Application.h"

const QUrl CURSEFORGE_APP_URL{ "https://curseforge.overwolf.com/electron/linux/CurseForge-0.198.1-21.AppImage" };

constexpr uint32_t IN_ADDR{ 82926761 };
constexpr uint32_t IN_SIZE{ 84196 };
constexpr uint32_t OUT_SIZE{ 131072 };

void FetchFlameAPIKey::executeTask()
{
    QNetworkRequest req{ CURSEFORGE_APP_URL };

    const auto& rangeHeader = QString("bytes=%1-%2").arg(IN_ADDR).arg(IN_ADDR + IN_SIZE);
    req.setRawHeader("Range", rangeHeader.toUtf8());

    m_reply.reset(APPLICATION->network()->get(req));
    connect(m_reply.get(), &QNetworkReply::downloadProgress, this, &Task::setProgress);
    connect(m_reply.get(), &QNetworkReply::finished, this, &FetchFlameAPIKey::downloadFinished);
    connect(m_reply.get(), &QNetworkReply::errorOccurred, this, [this](const QNetworkReply::NetworkError) {
        qCritical() << "Could not fetch official Flame API key:" << m_reply->errorString();
        emitFailed(m_reply->errorString());
    });

    setStatus(tr("Fetching API key from CurseForge app"));
}

bool FetchFlameAPIKey::abort()
{
    if (m_reply) {
        m_reply->abort();
    }
    emitAborted();
    return true;
}

void FetchFlameAPIKey::downloadFinished()
{
    auto res = m_reply->readAll();

    QByteArray expectedSizeHeader;
    QDataStream expectedSizeHeaderStream{ &expectedSizeHeader, QIODevice::WriteOnly };
    expectedSizeHeaderStream.setByteOrder(QDataStream::BigEndian);
    expectedSizeHeaderStream << OUT_SIZE;

    res.prepend(expectedSizeHeader);

    const auto& block = qUncompress(res);
    if (block.isEmpty()) {
        emitFailed("Couldn't decompress Curseforge app data.");
        return;
    }

    const auto precedingString = "\"cfCoreApiKey\":\"";
    const QByteArray preceding{ precedingString };
    const auto& precedingIndex = block.indexOf(preceding);
    if (precedingIndex == -1) {
        emitFailed(QString("Couldn't find string '%1'.").arg(precedingString));
        return;
    }

    const auto& startIndex = precedingIndex + preceding.size();
    const auto& finalIndex = block.indexOf(QByteArray{ "\"" }, startIndex);
    if (finalIndex == -1) {
        emitFailed("Couldn't find closing \" for cfCoreApiKey value.");
        return;
    }

    const auto& keyByteArray = block.mid(startIndex, finalIndex - startIndex);
    m_result = QString{ keyByteArray };
    qDebug() << "Fetched Flame API key:" << m_result;
    emitSucceeded();
}
