#include "PatternSearch.h"

#include <QRegularExpression>
#include <QtGlobal>

bool parseHexPattern(
    const QString& text,
    QVector<PatternByte>& pattern,
    QString* error)
{
    pattern.clear();

    const auto tokens =
        text.split(
            QRegularExpression("\\s+"),
            Qt::SkipEmptyParts);

    for (const QString& token : tokens) {
        if (token == "??") {
            pattern.push_back({0, true});
            continue;
        }

        bool ok = false;
        const int value = token.toInt(&ok, 16);

        if (!ok || value < 0 || value > 255) {
            if (error) {
                *error =
                    QString("Invalid HEX token: %1")
                        .arg(token);
            }

            pattern.clear();
            return false;
        }

        pattern.push_back({value, false});
    }

    if (pattern.isEmpty()) {
        if (error)
            *error = "Please enter at least one byte.";

        return false;
    }

    return true;
}

qint64 findPattern(
    const FileReader& file,
    const QVector<PatternByte>& pattern,
    qint64 startOffset)
{
    if (pattern.isEmpty() || !file.isOpen())
        return -1;

    constexpr qint64 chunkSize = 1024 * 1024;

    const qint64 overlap = pattern.size() - 1;

    QByteArray carry;

    for (qint64 pos = qMax<qint64>(0, startOffset);
         pos < file.size();) {

        const QByteArray block =
            file.readAt(pos, chunkSize);

        if (block.isEmpty())
            break;

        const QByteArray data = carry + block;
        const qint64 base = pos - carry.size();

        for (qsizetype i = 0;
             i + pattern.size() <= data.size();
             ++i) {

            bool match = true;

            for (qsizetype j = 0;
                 j < pattern.size();
                 ++j) {

                if (!pattern[j].wildcard &&
                    static_cast<unsigned char>(data[i + j])
                        != pattern[j].value) {

                    match = false;
                    break;
                }
            }

            if (match && base + i >= startOffset)
                return base + i;
        }

        const qsizetype keep =
            qMin<qint64>(overlap, data.size());

        carry = data.right(keep);

        pos += block.size();
    }

    return -1;
}

qint64 findBytes(
    const FileReader& file,
    const QByteArray& needle,
    qint64 startOffset)
{
    if (needle.isEmpty())
        return -1;

    QVector<PatternByte> pattern;
    pattern.reserve(needle.size());

    for (unsigned char c : needle)
        pattern.push_back({int(c), false});

    return ::findPattern(file, pattern, startOffset);
}
