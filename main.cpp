#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    app.setStyleSheet("QMainWindow { background-color: #333; } QLabel {color: #ddd; } QListView, QPushButton, QLineEdit, QTextBrowser {color: white; background-color: #111; }");
    MainWindow w;
    w.show();

    return app.exec();
}
