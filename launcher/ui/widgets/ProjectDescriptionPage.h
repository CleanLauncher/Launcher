#pragma once

#include <QTextBrowser>

#include "QObjectPtr.h"

QT_BEGIN_NAMESPACE
class VariableSizedImageObject;
QT_END_NAMESPACE

class ProjectDescriptionPage final : public QTextBrowser {
    Q_OBJECT

   public:
    ProjectDescriptionPage(QWidget* parent = nullptr);

    void setMetaEntry(QString entry);

   public slots:

    void flush();

   private:
    shared_qobject_ptr<VariableSizedImageObject> m_image_text_object;
};
