#include "nodeStorage.h"
#include "filesystem.h"
#include <iostream>
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    // Inicializar QCoreApplication
    QCoreApplication app(argc, argv);
    
    // Crear instancia de FileSystem
    FileSystem storage("sensors.bin");
    
    // Crear NodeStorage en puerto 5004
    NodeStorage storageNode(5004, &storage);
    storageNode.start();
    
    std::cout << "[System] Nodo Storage iniciado en puerto 5004.\n";
    std::cout << "[System] Escribe '#' para detenerlo.\n";
    
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
