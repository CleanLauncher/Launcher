#pragma once
#include <QDateTime>
#include <QString>

QDateTime timeFromS3Time(QString str);

QString timeToS3Time(QDateTime);
