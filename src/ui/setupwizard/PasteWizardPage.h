#ifndef PASTEDEFAULTSCONFIRMATIONWIZARD_H
#define PASTEDEFAULTSCONFIRMATIONWIZARD_H

#include "BaseWizardPage.h"
#include <QWidget>

namespace Ui
{
class PasteWizardPage;
}

class PasteWizardPage : public BaseWizardPage
{
    Q_OBJECT

public:
    explicit PasteWizardPage(QWidget* parent = nullptr);
    ~PasteWizardPage();

    void initializePage() override;
    bool validatePage() override;
    void retranslate() override;

private:
    Ui::PasteWizardPage* ui;
};

#endif
