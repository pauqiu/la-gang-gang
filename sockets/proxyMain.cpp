#include "nodeProxy.h"
#include "filesystem.h"
#include "endpoints.h"
#include <iostream>

int main() {
    FileSystem proxyFs("proxy_disk.bin");
    loadEndpoints("endpoints.txt");
    NodeProxy proxy(getProxyPort(), &proxyFs);
    proxy.start();

    std::cout << "[System] Nodo Proxy iniciado en puerto " << getProxyPort() << ". Escribe '#' para detenerlo.\n";
    std::cout << "IP -> " << getProxyIp() << std::endl;

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
