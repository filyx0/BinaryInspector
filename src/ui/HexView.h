#pragma once

#include <QAbstractScrollArea>

#include "../core/FileReader.h"

class HexView : public QAbstractScrollArea {
    Q_OBJECT

public:
    explicit HexView(QWidget* parent = nullptr);

    void setFile(const FileReader* file);

    qint64 selectedOffset() const {
        return selectedOffset_;
    }

    qint64 selectedLength() const {
        return selectedLength_;
    }

    QByteArray selectedBytes() const;
    void goToOffset(qint64 offset, qint64 length = 1);

signals:
    void selectionChanged(qint64 offset);

protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;

private:
    void updateScrollBar();

    const FileReader* file_ = nullptr;

    qint64 selectedOffset_ = -1;
    qint64 selectedLength_ = 0;

    int bytesPerRow_ = 16;
    int rowHeight_ = 20;
    int offsetWidth_ = 112;
    int hexStart_ = 126;
    int asciiStart_ = 126 + 16 * 25;
};
