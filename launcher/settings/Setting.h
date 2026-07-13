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

#include <QObject>
#include <QStringList>
#include <QVariant>
#include <memory>

class SettingsObject;

class Setting : public QObject {
    Q_OBJECT
   public:

    explicit Setting(QStringList synonyms, QVariant defVal = QVariant());

    virtual QString id() const { return m_synonyms.first(); }

    virtual QStringList configKeys() const { return m_synonyms; }

    virtual QVariant get() const;

    virtual QVariant defValue() const;

   signals:

    void SettingChanged(const Setting& setting, QVariant value);

    void settingReset(const Setting& setting);

   public slots:

    virtual void set(QVariant value);

    virtual void reset();

   protected:
    friend class SettingsObject;
    SettingsObject* m_storage;
    QStringList m_synonyms;
    QVariant m_defVal;
};
