#include "nodeReceptor.h"
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    // Inicializar QCoreApplication
    QCoreApplication app(argc, argv);
    
    // TODO: Add the logs file
    FileSystem receptor("ReceptorLogs.bin");
    
    // Crear NodeReceptor en puerto 9090
    NodeStorage storageNode(9090, &receptor);
    storageNode.start();
    
    std::cout << "[System] Nodo Storage iniciado en puerto 9090.\n";
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
