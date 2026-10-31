#include "Analysis.h"
#include <QCryptographicHash>
#include <QtEndian>
#include <cmath>
#include <cstring>

AnalysisReport analyzeFile(const FileReader& file)
{
    AnalysisReport r;
    r.size = file.size();
    if (!file.isOpen()) return r;

    QCryptographicHash hash(QCryptographicHash::Sha256);
    constexpr qint64 chunkSize = 1024 * 1024;
    for (qint64 pos = 0; pos < file.size(); pos += chunkSize) {
        const QByteArray chunk = file.readAt(pos, chunkSize);
        if (chunk.isEmpty()) break;
        hash.addData(chunk);
        for (unsigned char c : chunk)
            ++r.histogram[int(c)];
    }
    r.sha256 = hash.result();

    if (r.size > 0) {
        double h = 0.0;
        for (auto count : r.histogram) {
            const double p = double(count) / double(r.size);
            if (p > 0.0) h -= p * std::log2(p);
        }
        r.entropy = h;
    }

    const QByteArray head = file.readAt(0, 16);
    if (head.startsWith("MZ")) r.signatures << "DOS/Windows PE (MZ header)";
    if (head.startsWith("\x7F" "ELF")) r.signatures << "ELF executable";
    if (head.startsWith("PK\x03\x04")) r.signatures << "ZIP / DOCX / XLSX / JAR";
    if (head.startsWith("\x89PNG\r\n\x1A\n")) r.signatures << "PNG image";
    if (head.startsWith("GIF87a") || head.startsWith("GIF89a")) r.signatures << "GIF image";
    if (head.startsWith("\xFF\xD8\xFF")) r.signatures << "JPEG image";
    if (head.startsWith("%PDF-")) r.signatures << "PDF document";
    if (head.startsWith("SQLite format 3")) r.signatures << "SQLite database";
    if (r.signatures.isEmpty()) r.signatures << "No known signature detected";
    return r;
}

QString extractAsciiStrings(const FileReader& file, int minLength, qsizetype maxOutput)
{
    QString out;
    QByteArray current;
    qint64 start = 0;
    constexpr qint64 chunkSize = 256 * 1024;
    for (qint64 pos = 0; pos < file.size() && out.size() < maxOutput; pos += chunkSize) {
        const QByteArray chunk = file.readAt(pos, chunkSize);
        for (qsizetype i = 0; i < chunk.size(); ++i) {
            const unsigned char c = static_cast<unsigned char>(chunk[i]);
            if (c >= 0x20 && c <= 0x7E) {
                if (current.isEmpty()) start = pos + i;
                current.append(char(c));
            } else {
                if (current.size() >= minLength)
                    out += QString("0x%1  %2\n").arg(start, 8, 16, QLatin1Char('0'))
                               .arg(QString::fromLatin1(current));
                current.clear();
                if (out.size() >= maxOutput) break;
            }
        }
    }
    if (current.size() >= minLength && out.size() < maxOutput)
        out += QString("0x%1  %2\n").arg(start, 8, 16, QLatin1Char('0'))
                   .arg(QString::fromLatin1(current));
    if (out.size() >= maxOutput) out += "\n[Output truncated]\n";
    return out;
}

QString interpretBytes(const QByteArray& b)
{
    if (b.isEmpty()) return "No bytes selected";
    QString s;
    const int n = b.size();
    s += QString("Selected bytes: %1\nHEX: ").arg(n);
    for (unsigned char c : b) s += QString("%1 ").arg(c, 2, 16, QLatin1Char('0')).toUpper();
    if (n >= 2) {
        quint16 v = 0; memcpy(&v, b.constData(), 2);
        s += QString("\nuint16 LE: %1 | BE: %2").arg(qFromLittleEndian(v)).arg(qFromBigEndian(v));
    }
    if (n >= 4) {
        quint32 v = 0; memcpy(&v, b.constData(), 4);
        s += QString("\nuint32 LE: %1 | BE: %2").arg(qFromLittleEndian(v)).arg(qFromBigEndian(v));
    }
    if (n >= 8) {
        quint64 v = 0; memcpy(&v, b.constData(), 8);
        s += QString("\nuint64 LE: %1 | BE: %2").arg(qFromLittleEndian(v)).arg(qFromBigEndian(v));
    }
    return s;
}

QString formatSize(qint64 bytes)
{
    const double b = double(bytes);
    if (bytes < 1024) return QString("%1 B").arg(bytes);
    if (bytes < 1024LL * 1024) return QString("%1 KiB").arg(b / 1024.0, 0, 'f', 2);
    if (bytes < 1024LL * 1024 * 1024) return QString("%1 MiB").arg(b / (1024.0*1024.0), 0, 'f', 2);
    return QString("%1 GiB").arg(b / (1024.0*1024.0*1024.0), 0, 'f', 2);
}
