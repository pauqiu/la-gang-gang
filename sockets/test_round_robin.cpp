#include <iostream>
#include <thread>
#include <chrono>
#include "nodeClient.h"
#include "health_checker.h"

int main() {
    loadEndpoints("endpoints.txt");
    
    std::cout << "=== Test Round-Robin via Proxy ===\n\n";
    
    // 1. Verificar estado de nodos
    std::cout << "1. Estado de los nodos:\n";
    std::cout << "   Auth:     " << (HealthChecker::isNodeAlive(getAuthIp(), getAuthPort()) ? "ACTIVO" : "INACTIVO") << "\n";
    std::cout << "   Proxy:    " << (HealthChecker::isNodeAlive(getProxyIp(), getProxyPort()) ? "ACTIVO" : "INACTIVO") << "\n";
    std::cout << "   Storage1: " << (HealthChecker::isNodeAlive(getStorageIp(), getStoragePort()) ? "ACTIVO" : "INACTIVO") << "\n";
    if (hasStorage2()) {
        std::cout << "   Storage2: " << (HealthChecker::isNodeAlive(getStorage2Ip(), getStorage2Port()) ? "ACTIVO" : "INACTIVO") << "\n";
    }
    
    // 2. Autenticación
    std::cout << "\n2. Autenticando con Auth...\n";
    NodeClient client;
    
    if (!client.sendAuthentication("supervisor01", "visor_super777")) {
        std::cerr << "   ERROR: No se pudo autenticar\n";
        return 1;
    }
    std::cout << "   Autenticación enviada\n";
    
    // 3. Validar sesión con Proxy
    std::cout << "\n3. Validando sesión con Proxy...\n";
    if (!client.validateSessionWithProxy()) {
        std::cerr << "   ERROR: Sesión no válida\n";
        return 1;
    }
    std::cout << "   Sesión válida\n";
    
    // 4. Enviar solicitudes al Proxy (el Proxy usará round-robin internamente)
    std::cout << "\n4. Enviando 6 solicitudes al Proxy:\n";
    std::cout << "   (Observa los logs del Proxy para ver la alternancia)\n\n";
    
    int successful = 0;
    for (int i = 1; i <= 6; i++) {
        std::cout << "   [" << i << "] Solicitando lista de sensores... ";
        
        auto sensors = client.requestSensorList();
        std::cout << "OK (sensores: " << sensors.size() << ")\n";
        successful++;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    // 5. Resumen
    std::cout << "\n=== Resumen ===\n";
    std::cout << "Solicitudes exitosas: " << successful << "/6\n";
    std::cout << "\nRevisa los logs del Proxy para ver:\n";
    std::cout << "  'Storage seleccionado: 127.0.0.1:5004'\n";
    std::cout << "  'Storage seleccionado: 127.0.0.1:5005'\n";
    std::cout << "  alternando entre ambos storages.\n";
    
    return 0;
}
