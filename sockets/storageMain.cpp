#include "nodeStorage.h"
#include "filesystem.h"
#include "../include/raid_controller.h"
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
    
    // RAID 1: Crear dos discos para mirroring
    std::string diskPrimary = isSecondary ? "sensors2_disk1.bin" : "sensors_disk1.bin";
    std::string diskMirror = isSecondary ? "sensors2_disk2.bin" : "sensors_disk2.bin";
    
    FileSystem disk1(diskPrimary);
    FileSystem disk2(diskMirror);
    RaidController raid(&disk1, &disk2);
    
    NodeStorage<RaidController> storageNode(port, &raid);
    storageNode.start();
    
    std::cout << "[System] Nodo Storage" << (isSecondary ? "2" : "") 
              << " iniciado en puerto " << port << " con RAID 1.\n";
    std::cout << "IP -> " << ip << std::endl;
    std::cout << "[RAID1] Discos: " << diskPrimary << " (primario), " << diskMirror << " (espejo)\n";
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
