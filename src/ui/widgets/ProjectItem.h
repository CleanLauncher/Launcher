#pragma once

#include <QStyledItemDelegate>

enum UserDataTypes
{
    TITLE = 257,

    DESCRIPTION = 258,

    INSTALLED = 259

};

class ProjectItemDelegate final : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ProjectItemDelegate(QWidget* parent);

    void paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

signals:
    void checkboxClicked(const QModelIndex& index);

private:
    QStyleOptionViewItem makeCheckboxStyleOption(const QStyleOptionViewItem& opt, const QStyle* style) const;
};
