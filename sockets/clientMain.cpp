#include "nodeClient.h"
#include <thread>
#include <chrono>

int main() {
    NodeClient client;
    
    std::cout << "PRUEBA DEL SISTEMA DE AUTENTICACIÓN\n";
    std::cout << "---------------------------------------------------\n";

    // Test 1: Usuario válido "admin"
    std::cout << "TEST 1: Autenticación con usuario 'admin' (ÉXITO ESPERADO)\n";
    client.sendAuthentication("admin", "hashedpassword123", 0);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "---------------------------------------------------\n";

    // Test 2: Credenciales incorrectas "fail"
    std::cout << "TEST 2: Autenticación con usuario 'fail' (ERROR 1)\n";
    client.sendAuthentication("fail", "wrongpass", 0);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "---------------------------------------------------\n";

    // Test 3: Usuario bloqueado "blocked"
    std::cout << "TEST 3: Autenticación con usuario 'blocked' (ERROR 2)\n";
    client.sendAuthentication("blocked", "anypass", 0);
    
 

    return 0;
}