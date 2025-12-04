#include "filesystem.h"
#include "encryptation.h"
#include "security.h"
#include "RightsValidation/RightsValidation.h"
#include "mainwindow.h"

#include <QApplication>

#include <fstream>
#include <iostream>
#include <vector>
#include <tuple>
#include <string>

int main(int argc, char *argv[])
{
    const std::string diskName = "disk.bin";
    FileSystem fs(diskName);

    Security security(&fs);
    RightsValidation rights(&fs);

    if (!rights.initialize()) {
        std::cerr << "Warning: RightsValidation::initialize() devolvió false\n";
    }

    QApplication app(argc, argv);

    MainWindow mainWindow(&security, &rights);
    mainWindow.show();

    return app.exec();
}
