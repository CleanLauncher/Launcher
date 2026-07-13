#pragma once
#include <QIcon>
#include <QWidget>

class QStyleOption;

class IconLabel : public QWidget {
    Q_OBJECT

   public:
    explicit IconLabel(QWidget* parent, QIcon icon, QSize size);

    virtual QSize sizeHint() const;
    virtual void paintEvent(QPaintEvent*);

    void setIcon(QIcon icon);

   private:
    QSize m_size;
    QIcon m_icon;
};
