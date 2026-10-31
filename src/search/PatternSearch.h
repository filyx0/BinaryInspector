#pragma once

#include <QByteArray>
#include <QString>
#include <QVector>

#include "../core/FileReader.h"

struct PatternByte {
    int value = 0;
    bool wildcard = false;
};

bool parseHexPattern(
    const QString& text,
    QVector<PatternByte>& pattern,
    QString* error = nullptr);

qint64 findPattern(
    const FileReader& file,
    const QVector<PatternByte>& pattern,
    qint64 startOffset = 0);

qint64 findBytes(
    const FileReader& file,
    const QByteArray& needle,
    qint64 startOffset = 0);