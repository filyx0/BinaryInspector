#pragma once
#include <QFile>
#include <QString>
#include <QByteArray>
#include <memory>

class FileReader {
public:
    FileReader() = default;
    explicit FileReader(const QString& path) { open(path); }

    bool open(const QString& path);
    void close();
    bool isOpen() const;
    qint64 size() const;
    QString fileName() const;
    QByteArray readAt(qint64 offset, qint64 length) const;

private:
    std::unique_ptr<QFile> file_;
};
