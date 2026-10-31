#pragma once

#include <QMainWindow>
#include <QString>
#include <QVector>

#include "../core/FileReader.h"
#include "../plugins/PluginManager.h"
#include "../search/PatternSearch.h"

class HexView;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void openFile();
    void findPattern();
    void findNext();
    void goToOffset();
    void showAnalysis();
    void extractStrings();
    void compareFiles();
    void updateSelection(qint64 offset);

private:
    FileReader file_;
    HexView* hexView_ = nullptr;
    QLabel* status_ = nullptr;
    QLabel* selection_ = nullptr;
    PluginManager plugins_;

    QString lastSearchMode_;
    QString lastSearchText_;
    QVector<PatternByte> lastHexPattern_;
    QByteArray lastBytePattern_;
};
