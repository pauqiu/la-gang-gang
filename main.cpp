#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    loadEndpoints("endpoints.txt");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
