#include "nodeStorage.h"
#include "filesystem.h"
#include "endpoints.h"
#include <iostream>
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    loadEndpoints("endpoints.txt");
    
    // Determinar si es storage primario o secundario
    bool isSecondary = (argc > 1 && std::string(argv[1]) == "2");
    
    if (isSecondary && !hasStorage2()) {
        std::cerr << "[Error] STORAGE2 no está configurado en endpoints.txt\n";
        return 1;
    }
    
    std::string ip = isSecondary ? getStorage2Ip() : getStorageIp();
    int port = isSecondary ? getStorage2Port() : getStoragePort();
    std::string diskFile = isSecondary ? "sensors2.bin" : "sensors.bin";
    
    FileSystem storage(diskFile);
    NodeStorage storageNode(port, &storage);
    storageNode.start();
    
    std::cout << "[System] Nodo Storage" << (isSecondary ? "2" : "") 
              << " iniciado en puerto " << port << ".\n";
    std::cout << "IP -> " << ip << std::endl;
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
