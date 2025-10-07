#include "nodeAuth.h"
#include <iostream>

int main() {
    NodeAuth auth(5001);
    auth.start();

    std::cout << "[System] Nodo Auth iniciado. Escribe '#' para detenerlo.\n";

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
