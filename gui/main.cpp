#include "mainwindow.h"
#include "security.h"
#include "RightsValidation.h"
#include "filesystem.h"
#include <iostream>

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FileSystem * fs = new FileSystem("disk.bin");
    Security * sec = new Security (fs);
    RightsValidation rights(fs);

    MainWindow w(sec);
    w.show();
    return a.exec();
}
