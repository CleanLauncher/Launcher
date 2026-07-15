#pragma once

#include "BaseWizardPage.h"

class JavaWizardWidget;

class JavaWizardPage : public BaseWizardPage {
    Q_OBJECT
   public:
    explicit JavaWizardPage(QWidget* parent = Q_NULLPTR);

    virtual ~JavaWizardPage() = default;

    bool wantsRefreshButton() override;
    void refresh() override;
    void initializePage() override;
    bool validatePage() override;

   protected:
    void setupUi();
    void retranslate() override;

   private:
    JavaWizardWidget* m_java_widget = nullptr;
};
