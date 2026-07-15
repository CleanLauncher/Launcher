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

#include <QDialog>

#include <memory>

#include "minecraft/auth/AccountList.h"

namespace Ui {
class ProfileSelectDialog;
}

class ProfileSelectDialog : public QDialog {
    Q_OBJECT
   public:
    enum Flags {
        NoFlags = 0,

        GlobalDefaultCheckbox,

        InstanceDefaultCheckbox,
    };

    explicit ProfileSelectDialog(const QString& message = "", int flags = 0, QWidget* parent = 0);
    ~ProfileSelectDialog();

    MinecraftAccountPtr selectedAccount() const;

    bool useAsGlobalDefault() const;

    bool useAsInstDefaullt() const;

   public slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

   protected:
    AccountList* m_accounts;

    MinecraftAccountPtr m_selected;

   private:
    Ui::ProfileSelectDialog* ui;
};
