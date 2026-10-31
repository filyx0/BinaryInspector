#include "FileReader.h"

bool FileReader::open(const QString& path)
{
    close();
    auto candidate = std::make_unique<QFile>(path);
    if (!candidate->open(QIODevice::ReadOnly))
        return false;
    file_ = std::move(candidate);
    return true;
}

void FileReader::close() { if (file_) file_->close(); file_.reset(); }
bool FileReader::isOpen() const { return file_ && file_->isOpen(); }
qint64 FileReader::size() const { return isOpen() ? file_->size() : 0; }
QString FileReader::fileName() const { return isOpen() ? file_->fileName() : QString{}; }

QByteArray FileReader::readAt(qint64 offset, qint64 length) const
{
    if (!isOpen() || offset < 0 || length <= 0 || offset >= size())
        return {};
    if (!file_->seek(offset))
        return {};
    return file_->read(qMin(length, size() - offset));
}
