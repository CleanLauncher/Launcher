#pragma once

#include <QTextLayout>

QList<std::pair<qreal, QString>> viewItemTextLayout(QTextLayout& textLayout, int lineWidth, qreal& height);
