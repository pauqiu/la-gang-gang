#include "nodeReceptor.h"
#include "filesystem.h"
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    // Inicializar QCoreApplication
    QCoreApplication app(argc, argv);
    
    FileSystem receptor("rLogs.bin");
    
    // Crear NodeReceptor en puerto 9090
    nodeReceptor receptorNode(9090, &receptor);
    receptorNode.start();
    
    std::cout << "[System] Nodo Receptor iniciado en puerto 9090.\n";
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
