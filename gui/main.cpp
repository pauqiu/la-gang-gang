#include "mainwindow.h"
#include "security.h"
#include "../include/RightsValidation/RightsValidation.h"
#include "filesystem.h"
#include <iostream>

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FileSystem * fs = new FileSystem("disk.bin");
    Security * sec = new Security (fs);
    RightsValidation * rights = new RightsValidation(fs);

    sec->registerUser("Dylan", "123", "Admin");
    rights->addRole(1, "Administrador");
    rights->addPermissions(1, "read,write,delete,execute");


    if (!rights->initialize()) {
        std::cerr << "Error initializing Rights Validation system." << std::endl;
        return -1;
    }

    MainWindow w(sec, rights);
    w.show();
    return a.exec();
}
