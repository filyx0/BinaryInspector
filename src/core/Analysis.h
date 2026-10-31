#pragma once
#include "FileReader.h"
#include <QMap>
#include <QStringList>

struct AnalysisReport {
    qint64 size = 0;
    double entropy = 0.0;
    QMap<int, quint64> histogram;
    QByteArray sha256;
    QStringList signatures;
};

AnalysisReport analyzeFile(const FileReader& file);
QString extractAsciiStrings(const FileReader& file, int minLength = 4,
                            qsizetype maxOutput = 200000);
QString interpretBytes(const QByteArray& bytes);
QString formatSize(qint64 bytes);
