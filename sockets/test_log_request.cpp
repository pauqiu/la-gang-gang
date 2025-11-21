#include "nodeClient.h"
#include "endpoints.h"
#include "messages.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    try {
        // 1. Cargar endpoints
        // Asumimos que endpoints.txt está en el directorio actual o en el padre
        // Intentamos cargar desde el directorio actual primero
        try {
            loadEndpoints("endpoints.txt");
        } catch (...) {
            // Si falla, intentamos desde el directorio padre (común si se ejecuta desde build/)
            try {
                loadEndpoints("../endpoints.txt");
            } catch (...) {
                 std::cerr << "Error: No se pudo cargar endpoints.txt\n";
                 return 1;
            }
        }
        std::cout << "Endpoints cargados correctamente.\n";
        
        NodeClient client;
        
        std::cout << "INICIANDO TEST DE SOLICITUD DE LOGS\n";
        std::cout << "---------------------------------------------------\n";

        // 2. Autenticación
        // Usamos credenciales de prueba (asegúrate de que el usuario exista en la BD/Auth)
        std::string user = "supervisor01";
        std::string pass = "visor_super777";
        
        std::cout << "1. Autenticando usuario '" << user << "'...\n";
        if (client.sendAuthentication(user, pass, 0)) {
            std::cout << "Solicitud de autenticación enviada.\n";
            
            // Esperar a que el token se propague al Proxy
            std::cout << "Esperando propagación de token...\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            if (client.hasValidToken()) {
                std::cout << "Token recibido y válido.\n";
                
                // 3. Validar sesión con proxy (opcional, pero bueno para verificar)
                if (client.validateSessionWithProxy()) {
                    std::cout << "Sesión validada con Proxy.\n";
                    
                    // 4. Solicitar Logs
                    // Definimos un rango amplio para asegurar que traiga logs
                    uint64_t startDate = 20250101;
                    uint64_t endDate = 20251231;
                    
                    std::cout << "\n2. Solicitando logs del Proxy (Node Type: " << (int)NODE_PROXY 
                              << ", Range: " << startDate << " - " << endDate << ")...\n";
                    
                    std::vector<std::string> logs = client.requestNodeLogs(NODE_PROXY, startDate, endDate);
                    
                    if (logs.empty()) {
                        std::cout << "No se recibieron logs (o la lista está vacía).\n";
                    } else {
                        std::cout << "Test Exitoso: Se recibieron " << logs.size() << " logs.\n";
                    }
                } else {
                    std::cerr << "Fallo al validar sesión con Proxy.\n";
                }
            } else {
                std::cerr << "No se recibió token de autenticación. Verifique que el nodo Auth esté corriendo.\n";
            }
        } else {
            std::cerr << "Fallo al enviar solicitud de autenticación.\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Excepción: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
