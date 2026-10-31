#include "MainWindow.h"
#include "HexView.h"

#include "../core/Analysis.h"
#include "../search/PatternSearch.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QTextEdit>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QApplication>
#include <QLabel>
#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QKeySequence>

static void showTextDialog(
    QWidget* parent,
    const QString& title,
    const QString& text)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(title);
    dialog.resize(760, 520);

    auto* layout = new QVBoxLayout(&dialog);

    auto* edit = new QTextEdit(&dialog);
    edit->setReadOnly(true);
    edit->setPlainText(text);
    layout->addWidget(edit);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Close, &dialog);

    QObject::connect(
        buttons,
        &QDialogButtonBox::rejected,
        &dialog,
        &QDialog::reject);

    QObject::connect(
        buttons,
        &QDialogButtonBox::accepted,
        &dialog,
        &QDialog::accept);

    layout->addWidget(buttons);
    dialog.exec();
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("BinaryInspector");
    resize(1180, 720);

    hexView_ = new HexView(this);
    setCentralWidget(hexView_);

    status_ = new QLabel("No file open");
    selection_ = new QLabel("Selection: —");

    statusBar()->addWidget(status_, 1);
    statusBar()->addPermanentWidget(selection_);

    auto* fileMenu = menuBar()->addMenu("&File");

    auto* openAction = fileMenu->addAction("&Open...");
    openAction->setShortcut(QKeySequence::Open);

    connect(
        openAction,
        &QAction::triggered,
        this,
        &MainWindow::openFile);

    auto* compareAction =
        fileMenu->addAction("&Compare with...");

    connect(
        compareAction,
        &QAction::triggered,
        this,
        &MainWindow::compareFiles);

    fileMenu->addSeparator();

    auto* exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);

    connect(
        exitAction,
        &QAction::triggered,
        qApp,
        &QApplication::quit);

    auto* searchMenu = menuBar()->addMenu("&Search");

    auto* findAction =
        searchMenu->addAction("&Find pattern...");
    findAction->setShortcut(QKeySequence::Find);

    connect(
        findAction,
        &QAction::triggered,
        this,
        &MainWindow::findPattern);

    auto* findNextAction =
        searchMenu->addAction("Find &next");
    findNextAction->setShortcut(QKeySequence::FindNext);

    connect(
        findNextAction,
        &QAction::triggered,
        this,
        &MainWindow::findNext);

    auto* goToAction =
        searchMenu->addAction("&Go to offset...");
    goToAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));

    connect(
        goToAction,
        &QAction::triggered,
        this,
        &MainWindow::goToOffset);

    auto* analyzeMenu = menuBar()->addMenu("&Analyze");

    auto* analysisAction =
        analyzeMenu->addAction("File &analysis");

    connect(
        analysisAction,
        &QAction::triggered,
        this,
        &MainWindow::showAnalysis);

    auto* stringsAction =
        analyzeMenu->addAction("Extract &strings");

    connect(
        stringsAction,
        &QAction::triggered,
        this,
        &MainWindow::extractStrings);

    auto* pluginsAction =
        analyzeMenu->addAction("Run &plugins");

    connect(
        pluginsAction,
        &QAction::triggered,
        this,
        [this] {
            const auto results =
                plugins_.runAll(file_.fileName());

            showTextDialog(
                this,
                "Plugin results",
                results.isEmpty()
                    ? "No plugins loaded. Directory: plugins/"
                    : results.join("\n\n"));
        });

    auto* toolbar = addToolBar("Main");

    toolbar->addAction(
        "Open",
        this,
        &MainWindow::openFile);

    toolbar->addAction(
        "Find",
        this,
        &MainWindow::findPattern);

    toolbar->addAction(
        "Find Next",
        this,
        &MainWindow::findNext);

    toolbar->addAction(
        "Go To",
        this,
        &MainWindow::goToOffset);

    toolbar->addAction(
        "Analyze",
        this,
        &MainWindow::showAnalysis);

    toolbar->addAction(
        "Strings",
        this,
        &MainWindow::extractStrings);

    connect(
        hexView_,
        &HexView::selectionChanged,
        this,
        &MainWindow::updateSelection);

    plugins_.loadFrom(
        QApplication::applicationDirPath() + "/plugins");
}

void MainWindow::openFile()
{
    const QString path =
        QFileDialog::getOpenFileName(
            this,
            "Open binary file");

    if (path.isEmpty())
        return;

    if (!file_.open(path)) {
        QMessageBox::critical(
            this,
            "Open failed",
            "Could not open file for reading.");
        return;
    }

    hexView_->setFile(&file_);

    status_->setText(
        QString("%1 — %2")
            .arg(QFileInfo(path).fileName(),
                 formatSize(file_.size())));

    selection_->setText("Selection: —");

    setWindowTitle(
        QString("BinaryInspector — %1")
            .arg(QFileInfo(path).fileName()));
}

void MainWindow::findPattern()
{
    if (!file_.isOpen()) {
        QMessageBox::information(
            this,
            "Search",
            "Please open a file first.");
        return;
    }

    bool ok = false;

    const QStringList modes = {
        "ASCII / UTF-8",
        "HEX"
    };

    const QString mode =
        QInputDialog::getItem(
            this,
            "Find pattern",
            "Mode:",
            modes,
            lastSearchMode_ == modes[1] ? 1 : 0,
            false,
            &ok);

    if (!ok)
        return;

    const QString text =
        QInputDialog::getText(
            this,
            "Find pattern",
            mode.startsWith("HEX")
                ? "Pattern (e.g. 4D 5A ?? 00):"
                : "Text to search:",
            QLineEdit::Normal,
            lastSearchText_,
            &ok);

    if (!ok || text.isEmpty())
        return;

    lastSearchMode_ = mode;
    lastSearchText_ = text;

    qint64 found = -1;
    qint64 matchLength = 1;

    if (mode.startsWith("HEX")) {
        QVector<PatternByte> pattern;
        QString error;

        if (!parseHexPattern(text, pattern, &error)) {
            QMessageBox::warning(
                this,
                "Invalid pattern",
                error);
            return;
        }

        lastHexPattern_ = pattern;
        matchLength = pattern.size();
        found = ::findPattern(file_, pattern, 0);
    }
    else {
        const QByteArray needle = text.toUtf8();
        lastBytePattern_ = needle;
        matchLength = needle.size();
        found = findBytes(file_, needle, 0);
    }

    if (found < 0) {
        QMessageBox::information(
            this,
            "Search",
            "No match found.");
    }
    else {
        hexView_->goToOffset(found, matchLength);
        statusBar()->showMessage(
            QString("Found match at offset 0x%1 (%2 bytes)")
                .arg(found, 0, 16)
                .toUpper()
                .arg(matchLength),
            5000);
    }
}

void MainWindow::findNext()
{
    if (!file_.isOpen()) {
        QMessageBox::information(
            this,
            "Search",
            "Please open a file first.");
        return;
    }

    if (lastSearchText_.isEmpty()) {
        findPattern();
        return;
    }

    const qint64 startFrom = hexView_->selectedOffset() + 1;
    qint64 found = -1;
    qint64 matchLength = 1;

    if (lastSearchMode_.startsWith("HEX")) {
        matchLength = lastHexPattern_.size();
        found = ::findPattern(file_, lastHexPattern_, startFrom);
    }
    else {
        matchLength = lastBytePattern_.size();
        found = findBytes(file_, lastBytePattern_, startFrom);
    }

    if (found < 0) {
        QMessageBox::information(
            this,
            "Search",
            "No further occurrences found.");
    }
    else {
        hexView_->goToOffset(found, matchLength);
        statusBar()->showMessage(
            QString("Found next match at offset 0x%1 (%2 bytes)")
                .arg(found, 0, 16)
                .toUpper()
                .arg(matchLength),
            5000);
    }
}

void MainWindow::goToOffset()
{
    if (!file_.isOpen()) {
        QMessageBox::information(
            this,
            "Go to Offset",
            "Please open a file first.");
        return;
    }

    bool ok = false;
    const QString input =
        QInputDialog::getText(
            this,
            "Go to Offset",
            "Offset (hex e.g. 0x1A0 or decimal):",
            QLineEdit::Normal,
            {},
            &ok);

    if (!ok || input.trimmed().isEmpty())
        return;

    const QString trimmed = input.trimmed();
    qint64 target = -1;

    if (trimmed.startsWith("0x", Qt::CaseInsensitive)) {
        target = trimmed.mid(2).toLongLong(&ok, 16);
    }
    else {
        target = trimmed.toLongLong(&ok, 10);
        if (!ok)
            target = trimmed.toLongLong(&ok, 16);
    }

    if (!ok || target < 0 || target >= file_.size()) {
        QMessageBox::warning(
            this,
            "Go to Offset",
            QString("Offset out of range. File size is 0x%1 (%2 bytes).")
                .arg(file_.size(), 0, 16)
                .toUpper()
                .arg(file_.size()));
        return;
    }

    hexView_->goToOffset(target, 1);
    statusBar()->showMessage(
        QString("Jumped to offset 0x%1")
            .arg(target, 0, 16)
            .toUpper(),
        5000);
}

void MainWindow::showAnalysis()
{
    if (!file_.isOpen())
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    const AnalysisReport r = analyzeFile(file_);

    QApplication::restoreOverrideCursor();

    QString text =
        QString(
            "File: %1\n"
            "Size: %2 (%3 bytes)\n"
            "SHA-256: %4\n"
            "Entropy: %5 / 8.0 bits per byte\n\n"
            "Signatures:\n")
        .arg(file_.fileName(), formatSize(r.size))
        .arg(r.size)
        .arg(QString::fromLatin1(r.sha256.toHex()))
        .arg(r.entropy, 0, 'f', 4);

    for (const auto& sig : r.signatures)
        text += " * " + sig + "\n";

    text += "\nByte histogram (value: count):\n";

    for (auto it = r.histogram.cbegin();
         it != r.histogram.cend();
         ++it) {

        text += QString("%1: %2\n")
            .arg(it.key(), 2, 16, QLatin1Char('0'))
            .arg(it.value())
            .toUpper();
    }

    showTextDialog(
        this,
        "File analysis",
        text);
}

void MainWindow::extractStrings()
{
    if (!file_.isOpen())
        return;

    bool ok = false;

    const int minLen =
        QInputDialog::getInt(
            this,
            "Extract strings",
            "Minimum length:",
            4,
            2,
            1024,
            1,
            &ok);

    if (!ok)
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    const QString result =
        extractAsciiStrings(file_, minLen);

    QApplication::restoreOverrideCursor();

    showTextDialog(
        this,
        "Extracted strings",
        result);
}

void MainWindow::compareFiles()
{
    if (!file_.isOpen()) {
        QMessageBox::information(
            this,
            "Compare",
            "Please open the main file first.");
        return;
    }

    const QString path =
        QFileDialog::getOpenFileName(
            this,
            "Select file to compare");

    if (path.isEmpty())
        return;

    FileReader other(path);

    if (!other.isOpen()) {
        QMessageBox::warning(
            this,
            "Compare",
            "Could not open the second file.");
        return;
    }

    const auto a = analyzeFile(file_);
    const auto b = analyzeFile(other);

    const QString result =
        QString(
            "File A: %1\n"
            "Size: %2\n"
            "SHA-256: %3\n\n"
            "File B: %4\n"
            "Size: %5\n"
            "SHA-256: %6\n\n"
            "Result: %7")
        .arg(file_.fileName())
        .arg(a.size)
        .arg(QString::fromLatin1(a.sha256.toHex()))
        .arg(path)
        .arg(b.size)
        .arg(QString::fromLatin1(b.sha256.toHex()))
        .arg(
            a.size == b.size && a.sha256 == b.sha256
                ? "Identical files (hash and size match)"
                : "Different files");

    showTextDialog(
        this,
        "Compare files",
        result);
}

void MainWindow::updateSelection(qint64 offset)
{
    const QByteArray bytes =
        hexView_->selectedBytes();

    selection_->setText(
        QString("Offset: 0x%1 | %2")
            .arg(offset, 0, 16)
            .toUpper()
            .arg(bytes.left(8).toHex(' ').toUpper()));

    status_->setToolTip(
        interpretBytes(bytes));
}
