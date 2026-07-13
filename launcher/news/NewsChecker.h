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

#include <QList>
#include <QObject>
#include <QString>

#include <net/NetJob.h>

#include "NewsEntry.h"

class NewsChecker : public QObject {
    Q_OBJECT
   public:
    NewsChecker(QNetworkAccessManager* network, const QString& feedUrl);

    QString getLastLoadErrorMsg() const;

    bool isNewsLoaded() const;

    bool isLoadingNews() const;

    QList<NewsEntryPtr> getNewsEntries() const;

    void Q_SLOT reloadNews(const QString& newUrl = {});

   signals:

    void newsLoaded();

    void newsLoadingFailed(QString errorMsg);

   protected slots:
    void rssDownloadFinished();
    void rssDownloadFailed(QString reason);

   protected:
    QString m_feedUrl;

    QList<NewsEntryPtr> m_newsEntries;

    NetJob::Ptr m_newsNetJob;

    bool m_loadedNews;

    MetaEntryPtr m_entry;

    QString m_lastLoadError;

    QNetworkAccessManager* m_network;

   protected slots:

    void succeed();

    void fail(const QString& errorMsg);
};
