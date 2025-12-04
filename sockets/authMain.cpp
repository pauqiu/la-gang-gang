#include "nodeAuth.h"
#include "security.h"
#include "filesystem.h"
#include "endpoints.h"
#include "DataInjection.h"
#include <iostream>
#include <fstream>
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    FileSystem storage("aLogs.bin");

    const std::string usersDisk = "disk.bin";
    bool firstRun = true;
    {
        std::ifstream f(usersDisk.c_str());
        firstRun = !f.good();
    }

    FileSystem users(usersDisk);
    bool injected = false;
    if (firstRun) {
        DataInjection::injectSampleData(users);
        injected = true;
    }

    Security security(&users);

    RightsValidation rights(&users);
    if (!rights.initialize()) {
        std::cerr << "Warning: RightsValidation::initialize() devolvió false\n";
    }

    loadEndpoints("endpoints.txt");

    NodeAuth auth(getAuthPort(), &security, &rights, &storage);
    auth.start();

    std::cout << "[System] Nodo Auth iniciado. Escribe '#' para detenerlo.\n";
    std::cout << "IP -> " << getAuthIp() << std::endl;
    std::cout << "[System] Usuarios cargados: " << security.getUsers().size() << "\n";

    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input == "#") {
            std::cout << "[System] Apagando nodo...\n";
            break;
        }
    }

    std::cout << "[System] Nodo cerrado correctamente.\n";
    return 0;
}
