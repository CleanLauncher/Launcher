// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Launcher: Minecraft Launcher
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

#include "NetworkCheck.h"

#include "Application.h"
#include "net/HttpMetaCache.h"
#include "settings/SettingsObject.h"

NetworkCheck::NetworkCheck(QNetworkAccessManager* network)
{
    m_network = network;

    static auto s_urlToResult = std::array{
        std::pair{ QUrl("https://prismlauncher.org"), Result::UsePrimary },
        std::pair{ QUrl("https://prismlauncher.org"), Result::UseNewFallback },
        std::pair{ QUrl("https://prismlauncher.org"), Result::UseOldFallback },
    };
    for (auto& [url, result] : s_urlToResult) {
        launchRequest(url, result);
    }
}

QMap<NetworkCheck::Result, QString> NetworkCheck::metaUrls()
{
    return {
        { Result::UsePrimary, "" },
        { Result::UseNewFallback, "https://prismlauncher.org/meta/v1/" },
        { Result::UseOldFallback, "https://prismlauncher.org/meta/v1/" },
    };
}

QMap<NetworkCheck::Result, QString> NetworkCheck::fmlLibsUrls()
{
    return {
        { Result::UsePrimary, "" },
        { Result::UseNewFallback, "https://prismlauncher.org/files/fmllibs/" },
        { Result::UseOldFallback, "https://prismlauncher.org/files/fmllibs/" },
    };
}

QMap<NetworkCheck::Result, QString> NetworkCheck::newsUrls()
{
    return {
        { Result::UsePrimary, "" },
        { Result::UseNewFallback, "https://prismlauncher.org/feed/feed.xml" },
        { Result::UseOldFallback, "https://prismlauncher.org/feed/feed.xml" },
    };
}

QMap<NetworkCheck::Result, QString> NetworkCheck::translationsUrls()
{
    return {
        { Result::UsePrimary, "" },
        { Result::UseNewFallback, "https://prismlauncher.org/i18n" },
        { Result::UseOldFallback, "https://prismlauncher.org/i18n" },
    };
}

void NetworkCheck::launchRequest(const QUrl& url, Result ifSuccess)
{
    QNetworkRequest request(url);
    request.setTransferTimeout(std::chrono::seconds(3));
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    auto* reply = m_network->head(request);
    m_pendingRequests++;

    qInfo() << "[NetworkCheck] Checking" << url;
    connect(reply, &QNetworkReply::finished, this, [this, reply, ifSuccess] {
        m_pendingRequests--;

        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qInfo() << "[NetworkCheck]" << reply->url() << "result:" << reply->error() << status;
        if (reply->error() == QNetworkReply::NoError && status < 400 && ifSuccess < m_result) {
            m_result = ifSuccess;
        }
        reply->deleteLater();

        if (!m_finished && (m_pendingRequests == 0 || m_result == Result::UsePrimary)) {
            qInfo() << "[NetworkCheck] Final result:" << m_result;
            finished();
            m_finished = true;
        }
    });
}

bool NetworkCheck::handleUrlOverride(const QString& overrideName, const QMap<Result, QString>& urlMap) const
{
    if (!urlMap.contains(m_result)) {
        return false;
    }
    const QString newOverride = urlMap.value(m_result);

    auto* settings = APPLICATION->settings();
    const auto currentOverride = settings->get(overrideName).toString();
    if (currentOverride == newOverride) {
        return false;
    }
    if (!currentOverride.isEmpty() && !urlMap.values().contains(currentOverride)) {
        return false;
    }

    settings->set(overrideName, newOverride);
    qInfo() << "[NetworkCheck] Updated setting" << overrideName << "to" << newOverride;
    return true;
}

void NetworkCheck::finished()
{
    if (!APPLICATION->settings()->get("AutoServers").toBool()) {
        qInfo() << "[NetworkCheck] Automatic server switching is disabled";
        return;
    }

    if (handleUrlOverride("MetaURLOverride", metaUrls())) {
        if (!APPLICATION->metacache()->softEvict()) {
            qWarning() << "[NetworkCheck] Could not evict metacache during automatic meta switch";
        }
        APPLICATION->metacache()->SaveNow();
    }

    std::ignore = handleUrlOverride("LegacyFMLLibsURLOverride", fmlLibsUrls());

    if (const auto newsUrls = this->newsUrls(); newsUrls.contains(m_result)) {
        emit shouldReloadNews(newsUrls.value(m_result));
    }
}
