#include "nodeAuth.h"
#include "security.h"
#include "filesystem.h"
#include <iostream>
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    // Inicializar QCoreApplication (necesario para QString y Qt)
    QCoreApplication app(argc, argv);

    // Crear instancia de FileSystem
    FileSystem storage("disk.bin");

    // Crear instancia de Security con FileSystem
    Security security(&storage);

    // Pasar Security al NodeAuth
    NodeAuth auth(5001, &security);
    auth.start();

    std::cout << "[System] Nodo Auth iniciado. Escribe '#' para detenerlo.\n";
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
