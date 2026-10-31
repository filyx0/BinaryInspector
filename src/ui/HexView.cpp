#include "HexView.h"

#include <QPainter>
#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QFontDatabase>
#include <QFontMetrics>

#include <climits>
#include <algorithm>

HexView::HexView(QWidget* parent)
    : QAbstractScrollArea(parent)
{
    setFont(
        QFontDatabase::systemFont(
            QFontDatabase::FixedFont));

    setMinimumWidth(650);
    setFocusPolicy(Qt::StrongFocus);

    connect(
        verticalScrollBar(),
        &QScrollBar::valueChanged,
        this,
        [this](int) {
            viewport()->update();
        });

    updateScrollBar();
}

void HexView::setFile(const FileReader* file)
{
    file_ = file;
    selectedOffset_ = -1;
    selectedLength_ = 0;

    verticalScrollBar()->setValue(0);

    updateScrollBar();
    viewport()->update();
}

void HexView::goToOffset(qint64 offset, qint64 length)
{
    if (!file_ || !file_->isOpen() || offset < 0 || offset >= file_->size())
        return;

    selectedOffset_ = offset;
    selectedLength_ = qMax<qint64>(1, qMin(length, file_->size() - offset));

    const qint64 targetRow = offset / bytesPerRow_;
    const int visible = qMax(1, viewport()->height() / rowHeight_);
    const qint64 desiredRow =
        std::clamp<qint64>(targetRow - visible / 3, 0, verticalScrollBar()->maximum());

    verticalScrollBar()->setValue(static_cast<int>(desiredRow));

    emit selectionChanged(offset);
    viewport()->update();
}

QByteArray HexView::selectedBytes() const
{
    if (file_ && selectedOffset_ >= 0)
        return file_->readAt(selectedOffset_, qMax<qint64>(8, selectedLength_));

    return {};
}

void HexView::updateScrollBar()
{
    const qint64 rows = file_
        ? (file_->size() + bytesPerRow_ - 1) / bytesPerRow_
        : 0;

    const int visible =
        qMax(1, viewport()->height() / rowHeight_);

    const qint64 maxRows =
        qMax<qint64>(0, rows - visible);

    verticalScrollBar()->setRange(
        0,
        int(qMin<qint64>(INT_MAX, maxRows)));

    verticalScrollBar()->setPageStep(visible);
}

void HexView::resizeEvent(QResizeEvent* event)
{
    QAbstractScrollArea::resizeEvent(event);
    updateScrollBar();
}

void HexView::paintEvent(QPaintEvent*)
{
    QPainter painter(viewport());

    painter.fillRect(
        viewport()->rect(),
        QColor("#171a1f"));

    painter.setFont(font());

    if (!file_ || !file_->isOpen()) {
        painter.setPen(QColor("#8993a1"));

        painter.drawText(
            24,
            36,
            "Open a file to view its bytes.");

        return;
    }

    const int visible =
        viewport()->height() / rowHeight_ + 2;

    const qint64 firstRow =
        verticalScrollBar()->value();

    const qint64 firstOffset =
        firstRow * bytesPerRow_;

    const QByteArray rowData =
        file_->readAt(
            firstOffset,
            qint64(visible) * bytesPerRow_);

    const QFontMetrics metrics(font());

    const int charWidth =
        metrics.horizontalAdvance('0');

    hexStart_ = offsetWidth_ + 12;

    asciiStart_ =
        hexStart_ + bytesPerRow_ * (charWidth * 3 + 7);

    painter.setPen(QColor("#8b98a9"));

    painter.drawText(12, 16, "OFFSET");
    painter.drawText(hexStart_, 16, "HEX");
    painter.drawText(asciiStart_, 16, "ASCII");

    painter.setPen(QColor("#46505d"));
    painter.drawLine(
        0,
        23,
        viewport()->width(),
        23);

    for (int row = 0; row < visible; ++row) {
        const qint64 rowOffset =
            firstOffset + qint64(row) * bytesPerRow_;

        if (rowOffset >= file_->size())
            break;

        const int y = 24 + row * rowHeight_;
        const int local = row * bytesPerRow_;

        painter.setPen(QColor("#8da0b5"));

        painter.drawText(
            12,
            y + 14,
            QString("%1")
                .arg(
                    rowOffset,
                    8,
                    16,
                    QLatin1Char('0'))
                .toUpper());

        for (int col = 0; col < bytesPerRow_; ++col) {
            const int index =
                local + col;

            if (index >= rowData.size())
                break;

            const unsigned char byte =
                static_cast<unsigned char>(rowData[index]);

            const qint64 absolute =
                rowOffset + col;

            const int x =
                hexStart_ + col * (charWidth * 3 + 7);

            const bool isSelected =
                (selectedLength_ > 0)
                    ? (absolute >= selectedOffset_ &&
                       absolute < selectedOffset_ + selectedLength_)
                    : (absolute == selectedOffset_);

            if (isSelected) {
                painter.fillRect(
                    x - 2,
                    y,
                    charWidth * 2 + 5,
                    rowHeight_,
                    QColor("#31577d"));

                painter.fillRect(
                    asciiStart_ + col * charWidth - 1,
                    y,
                    charWidth + 2,
                    rowHeight_,
                    QColor("#31577d"));
            }

            painter.setPen(
                isSelected
                    ? QColor("#ffffff")
                    : QColor("#d6b879"));

            painter.drawText(
                x,
                y + 14,
                QString("%1")
                    .arg(
                        byte,
                        2,
                        16,
                        QLatin1Char('0'))
                    .toUpper());

            painter.setPen(
                isSelected
                    ? QColor("#ffffff")
                    : (byte >= 0x20 && byte <= 0x7e
                        ? QColor("#a8d5a2")
                        : QColor("#697584")));

            const QChar character(
                byte >= 0x20 && byte <= 0x7e
                    ? byte
                    : '.');

            painter.drawText(
                asciiStart_ + col * charWidth,
                y + 14,
                QString(character));
        }
    }
}

void HexView::mousePressEvent(QMouseEvent* event)
{
    if (!file_ ||
        !file_->isOpen() ||
        event->button() != Qt::LeftButton) {
        return;
    }

    const int y =
        int(event->position().y());

    if (y < 24)
        return;

    const int row =
        (y - 24) / rowHeight_;

    const int x =
        int(event->position().x());

    int col = -1;

    const QFontMetrics metrics(font());

    const int charWidth =
        metrics.horizontalAdvance('0');

    if (x >= hexStart_ && x < asciiStart_) {
        col =
            (x - hexStart_) / (charWidth * 3 + 7);
    }
    else if (x >= asciiStart_) {
        col =
            (x - asciiStart_) / charWidth;
    }

    if (col < 0 || col >= bytesPerRow_)
        return;

    const qint64 offset =
        (qint64(verticalScrollBar()->value()) + row)
        * bytesPerRow_ + col;

    if (offset < file_->size()) {
        selectedOffset_ = offset;
        selectedLength_ = 1;

        emit selectionChanged(offset);

        viewport()->update();
    }
}

void HexView::wheelEvent(QWheelEvent* event)
{
    verticalScrollBar()->setValue(
        verticalScrollBar()->value()
        - event->angleDelta().y() / 120 * 3);

    event->accept();
}
