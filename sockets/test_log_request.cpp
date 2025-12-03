#include "nodeClient.h"
#include "endpoints.h"
#include "messages.h"
#include <iostream>
#include <thread>
#include <chrono>

// Helper para obtener nombre del nodo
std::string getNodeName(uint8_t nodeType) {
    switch (nodeType) {
        case NODE_PROXY: return "PROXY";
        case NODE_AUTH: return "AUTH";
        case NODE_STORAGE: return "STORAGE";
        default: return "UNKNOWN";
    }
}

// Función para probar solicitud de logs a un nodo específico
void testLogRequest(NodeClient& client, uint8_t nodeType, uint64_t startDate, uint64_t endDate) {
    std::cout << "\n---------------------------------------------------\n";
    std::cout << "Solicitando logs del nodo " << getNodeName(nodeType) 
              << " (Type: " << (int)nodeType << ", Range: " << startDate << " - " << endDate << ")...\n";
    
    std::vector<std::string> logs = client.requestNodeLogs(nodeType, startDate, endDate);
    
    if (logs.empty()) {
        std::cout << "[" << getNodeName(nodeType) << "] No se recibieron logs (o la lista está vacía).\n";
    } else {
        std::cout << "[" << getNodeName(nodeType) << "] Test Exitoso: Se recibieron " << logs.size() << " logs.\n";
    }
}

int main() {
    try {
        // 1. Cargar endpoints
        try {
            loadEndpoints("endpoints.txt");
        } catch (...) {
            try {
                loadEndpoints("../endpoints.txt");
            } catch (...) {
                 std::cerr << "Error: No se pudo cargar endpoints.txt\n";
                 return 1;
            }
        }
        std::cout << "Endpoints cargados correctamente.\n";
        
        NodeClient client;
        
        std::cout << "===================================================\n";
        std::cout << "    TEST DE SOLICITUD DE LOGS A TODOS LOS NODOS\n";
        std::cout << "===================================================\n";

        // 2. Autenticación
        std::string user = "supervisor01";
        std::string pass = "visor_super777";
        
        std::cout << "\n1. Autenticando usuario '" << user << "'...\n";
        if (!client.sendAuthentication(user, pass, 0)) {
            std::cerr << "Fallo al enviar solicitud de autenticación.\n";
            return 1;
        }
        
        std::cout << "Esperando propagación de token...\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        if (!client.hasValidToken()) {
            std::cerr << "No se recibió token. Verifique que el nodo Auth esté corriendo.\n";
            return 1;
        }
        std::cout << "Token recibido y válido.\n";
        
        // 3. Validar sesión con proxy
        if (!client.validateSessionWithProxy()) {
            std::cerr << "Fallo al validar sesión con Proxy.\n";
            return 1;
        }
        std::cout << "Sesión validada con Proxy.\n";
        
        // 4. Definir rango de fechas
        uint64_t startDate = 20250101;
        uint64_t endDate = 20251231;
        
        // 5. Probar solicitud de logs a cada nodo
        std::cout << "\n2. Probando solicitud de logs a todos los nodos...\n";
        
        // Test PROXY
        testLogRequest(client, NODE_PROXY, startDate, endDate);
        
        // Test AUTH (conexión directa al nodo Auth)
        testLogRequest(client, NODE_AUTH, startDate, endDate);
        
        // Test STORAGE (conexión directa al nodo Storage)
        testLogRequest(client, NODE_STORAGE, startDate, endDate);
        
        std::cout << "\n===================================================\n";
        std::cout << "              TESTS COMPLETADOS\n";
        std::cout << "===================================================\n";

    } catch (const std::exception& e) {
        std::cerr << "Excepción: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
