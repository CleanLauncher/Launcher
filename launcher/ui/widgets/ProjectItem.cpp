#include "ProjectItem.h"

#include <QApplication>

#include <QDebug>
#include <QIcon>
#include <QPainter>
#include "Common.h"

ProjectItemDelegate::ProjectItemDelegate(QWidget* parent) : QStyledItemDelegate(parent) {}

void ProjectItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    auto isInstalled = index.data(UserDataTypes::INSTALLED).toBool();
    auto isChecked = opt.checkState == Qt::Checked;
    auto isSelected = option.state & QStyle::State_Selected;

    const QStyle* style = opt.widget == nullptr ? QApplication::style() : opt.widget->style();

    auto rect = opt.rect;

    bool windows = style->objectName().startsWith("windows");

    if (!windows)
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    if (isSelected) {
        if (windows)
            painter->fillRect(rect, opt.palette.highlight());

        painter->setPen(opt.palette.highlightedText().color());
    }

    if (opt.features & QStyleOptionViewItem::HasCheckIndicator) {
        QStyleOptionViewItem checkboxOpt = makeCheckboxStyleOption(opt, style);
        style->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &checkboxOpt, painter, opt.widget);

        rect.setX(checkboxOpt.rect.right());
    }

    if (!isSelected && !isChecked && isInstalled) {
        painter->setOpacity(0.4);
    }

    auto icon_width = rect.height();
    int icon_x_margin = (rect.height() - icon_width) / 2;

    if (!opt.icon.isNull()) {
        auto icon_height = 0;
        {
            auto icon_size = opt.decorationSize;
            icon_width = icon_size.width();
            icon_height = icon_size.height();

            icon_x_margin = (rect.height() - icon_height) / 2;
        }

        int x = rect.x() + icon_x_margin;
        int y = rect.y() + icon_x_margin;

        if (opt.features & QStyleOptionViewItem::HasCheckIndicator) {
            rect.translate(icon_x_margin / 2, 0);
        }

        if (icon_width > 0 && icon_height > 0) {
            opt.icon.paint(painter, x, y, icon_width, icon_height);
        }
    }

    auto remaining_width = rect.width() - icon_width - 2 * icon_x_margin;
    rect.setRect(rect.x() + icon_width + 2 * icon_x_margin, rect.y(), remaining_width, rect.height());

    int title_height = 0;

    {
        auto title = index.data(UserDataTypes::TITLE).toString();

        painter->save();

        auto font = opt.font;
        if (isChecked) {
            font.setBold(true);
        }
        if (isInstalled) {
            title = tr("%1 [installed]").arg(title);
        }

        font.setPointSize(font.pointSize() + 2);
        painter->setFont(font);

        title_height = QFontMetrics(font).height();

        painter->drawText(rect.x(), rect.y() + title_height, title);

        painter->restore();
    }

    {
        auto description = index.data(UserDataTypes::DESCRIPTION).toString().simplified();

        QTextLayout text_layout(description, opt.font);

        qreal height = 0;
        auto cut_text = viewItemTextLayout(text_layout, remaining_width, height);

        description = cut_text.first().second;
        auto num_lines = 1;

        if (cut_text.size() > 1) {
            if (rect.height() - title_height <= 2.5 * opt.fontMetrics.height()) {
                description = opt.fontMetrics.elidedText(description, opt.textElideMode, cut_text.at(0).first);
            } else {
                if (cut_text.size() > 2) {
                    description += opt.fontMetrics.elidedText(cut_text.at(1).second, opt.textElideMode, cut_text.at(1).first);
                } else {
                    description += cut_text.at(1).second;
                }
                num_lines += 1;
            }
        }

        int description_x = rect.x();

        int description_y = rect.y() + title_height + (rect.height() - title_height) / 2;
        if (num_lines == 1)
            description_y -= opt.fontMetrics.height() / 2;
        else
            description_y -= opt.fontMetrics.height();

        painter->drawText(description_x, description_y, remaining_width, num_lines * opt.fontMetrics.height(), Qt::TextWordWrap,
                          description);
    }

    painter->restore();
}

bool ProjectItemDelegate::editorEvent(QEvent* event,
                                      QAbstractItemModel* model,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index)
{
    if (!(event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseButtonPress ||
          event->type() == QEvent::MouseButtonDblClick))
        return false;

    auto mouseEvent = (QMouseEvent*)event;

    if (mouseEvent->button() != Qt::LeftButton)
        return false;

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    const QStyle* style = opt.widget == nullptr ? QApplication::style() : opt.widget->style();

    const QStyleOptionViewItem checkboxOpt = makeCheckboxStyleOption(opt, style);

    if (!checkboxOpt.rect.contains(mouseEvent->pos().x(), mouseEvent->pos().y()))
        return false;

    if (event->type() != QEvent::MouseButtonRelease)
        return true;

    emit checkboxClicked(index);
    return true;
}

QStyleOptionViewItem ProjectItemDelegate::makeCheckboxStyleOption(const QStyleOptionViewItem& opt, const QStyle* style) const
{
    QStyleOptionViewItem checkboxOpt = opt;

    checkboxOpt.state &= ~QStyle::State_HasFocus;

    if (checkboxOpt.checkState == Qt::Checked)
        checkboxOpt.state |= QStyle::State_On;
    else
        checkboxOpt.state |= QStyle::State_Off;

    QRect checkboxRect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &checkboxOpt, opt.widget);

    checkboxOpt.rect = QRect(opt.rect.x() + 5, opt.rect.y() + (opt.rect.height() / 2 - checkboxRect.height() / 2), checkboxRect.width(),
                             checkboxRect.height());

    return checkboxOpt;
}
