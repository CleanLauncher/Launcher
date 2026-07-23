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

#include <QDomElement>
#include <QObject>
#include <QString>
#include <memory>

class NewsEntry : public QObject
{
    Q_OBJECT

public:
    explicit NewsEntry(QObject* parent = 0);

    NewsEntry(const QString& title, const QString& content, const QString& link, QObject* parent = 0);

    static bool fromXmlElement(const QDomElement& element, NewsEntry* entry, QString* errorMsg = 0);

    QString title;

    QString content;

    QString link;
};

using NewsEntryPtr = std::shared_ptr<NewsEntry>;
