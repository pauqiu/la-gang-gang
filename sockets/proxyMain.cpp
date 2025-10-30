#include "nodeProxy.h"
#include <iostream>

int main() {
    NodeProxy proxy(5002);
    proxy.start();

    std::cout << "[System] Nodo Proxy iniciado en puerto 5002. Escribe '#' para detenerlo.\n";

    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input == "#") {
            std::cout << "[System] Apagando nodo Proxy...\n";
            break;
        }
    }

    std::cout << "[System] Nodo Proxy cerrado correctamente.\n";
    return 0;
}
