#include "ui/MainWindow.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName("BinaryInspector");
    QApplication::setOrganizationName("BinaryInspector");

    app.setStyle(QStyleFactory::create("Fusion"));

    app.setStyleSheet(R"(
        QWidget {
            background: #202329;
            color: #e5e7eb;
        }

        QMenuBar, QMenu, QToolBar {
            background: #292e36;
            color: #e5e7eb;
        }

        QMenu::item:selected {
            background: #3b4655;
        }

        QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox {
            background: #171a1f;
            color: #e5e7eb;
            border: 1px solid #454d59;
            selection-background-color: #345b83;
        }

        QPushButton {
            background: #343c47;
            border: 1px solid #505b69;
            padding: 5px 10px;
        }

        QPushButton:hover {
            background: #414d5b;
        }

        QHeaderView::section {
            background: #303741;
            padding: 4px;
            border: 0;
        }

        QScrollBar:vertical {
            background: #252a31;
            width: 13px;
        }

        QScrollBar::handle:vertical {
            background: #586575;
            min-height: 24px;
        }

        QStatusBar {
            background: #292e36;
        }
    )");

    MainWindow window;
    window.show();

    return app.exec();
}