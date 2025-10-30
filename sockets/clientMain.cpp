#include "nodeClient.h"
#include <thread>
#include <chrono>

int main() {
    NodeClient client;
    
    std::cout << "PRUEBA DEL SISTEMA DE AUTENTICACIÓN COMPLETO\n";
    std::cout << "---------------------------------------------------\n";

    // Test 1: Autenticación exitosa y validación con proxy
    std::cout << "Test1 1: Flujo completo con usuario 'admin'\n";
    std::cout << "---------------------------------------------------\n";
    client.sendAuthentication("admin", "hashedpassword123", 0);
    
    // Esperar a que el token se registre en el proxy
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Validar sesión con el proxy
    if (client.hasValidToken()) {
        std::cout << "\n[Client] Validando sesión con el Proxy...\n";
        client.validateSessionWithProxy();
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "---------------------------------------------------\n";

    // Test 2: Autenticación fallida (credenciales incorrectas)
    std::cout << "Test 2: Autenticación fallida con usuario 'fail'\n";
    std::cout << "---------------------------------------------------\n";
    NodeClient client2;
    client2.sendAuthentication("fail", "wrongpass", 0);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "---------------------------------------------------\n";

    // Test 3: Autenticación fallida (usuario bloqueado)
    std::cout << "Test 3: Usuario bloqueado 'blocked'\n";
    NodeClient client3;
    client3.sendAuthentication("blocked", "anypass", 0);

    return 0;
}