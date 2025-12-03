#include <iostream>
#include "health_checker.h"
#include "endpoints.h"

int main() {
    loadEndpoints("endpoints.txt");
    
    std::cout << "=== Test Health Check ===\n\n";
    
    std::cout << "Storage: " << (HealthChecker::isNodeAlive(getStorageIp(), getStoragePort()) ? "ACTIVO" : "INACTIVO") << "\n";
    std::cout << "Auth:    " << (HealthChecker::isNodeAlive(getAuthIp(), getAuthPort()) ? "ACTIVO" : "INACTIVO") << "\n";
    std::cout << "Proxy:   " << (HealthChecker::isNodeAlive(getProxyIp(), getProxyPort()) ? "ACTIVO" : "INACTIVO") << "\n";
    
    if (hasStorage2()) {
        std::cout << "Storage2: " << (HealthChecker::isNodeAlive(getStorage2Ip(), getStorage2Port()) ? "ACTIVO" : "INACTIVO") << "\n";
    }
    
    return 0;
}
