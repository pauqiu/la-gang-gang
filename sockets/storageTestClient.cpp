#include "nodeClient.h"
#include "messages.h"
#include "communication.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

class StorageTestClient {
public:
    // Test 1: Guardar datos
    void testStorageSave() {
        std::cout << "\n========================================\n";
        std::cout << "TEST 1: Guardar datos de sensor\n";
        std::cout << "========================================\n";
        
        int sock = connect_to("127.0.0.1", 5004);
        if (sock < 0) {
            std::cerr << "[TestClient] Error conectando a Storage\n";
            return;
        }
        
        // Crear mensaje de guardado
        StorageSave msg;
        msg.sensorId = 1;
        msg.date = 20241021;  // 2024-10-21
        msg.time = 143000;    // 14:30:00
        msg.dataLength = 10;
        msg.data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
        
        auto data = msg.serialize();
        
        std::cout << "[TestClient] Enviando StorageSave:\n";
        std::cout << "  - SensorId: " << (int)msg.sensorId << "\n";
        std::cout << "  - Date: " << msg.date << "\n";
        std::cout << "  - Time: " << msg.time << "\n";
        std::cout << "  - DataLength: " << (int)msg.dataLength << "\n";
        
        if (!send_message(sock, data.data(), data.size())) {
            std::cerr << "[TestClient] Error enviando mensaje\n";
            close(sock);
            return;
        }
        
        // Recibir respuesta
        std::vector<uint8_t> response(1024);
        ssize_t bytes = recv_message(sock, response.data(), response.size());
        
        if (bytes > 0) {
            response.resize(bytes);
            processStorageResponse(response);
        } else {
            std::cerr << "[TestClient] No se recibió respuesta\n";
        }
        
        close(sock);
    }
    
    // Test 2: Sincronizar datos
    void testStorageSync() {
        std::cout << "\n========================================\n";
        std::cout << "TEST 2: Sincronizar datos de sensor\n";
        std::cout << "========================================\n";
        
        // Esperar un poco para que se guarden los datos
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        int sock = connect_to("127.0.0.1", 5004);
        if (sock < 0) {
            std::cerr << "[TestClient] Error conectando a Storage\n";
            return;
        }
        
        // Crear mensaje de sincronización
        StorageSyncRequest msg;
        msg.sensorId = 1;
        msg.startDate = 20241020;  // Desde 2024-10-20
        msg.endDate = 20241022;    // Hasta 2024-10-22
        
        auto data = msg.serialize();
        
        std::cout << "[TestClient] Enviando StorageSyncRequest:\n";
        std::cout << "  - SensorId: " << (int)msg.sensorId << "\n";
        std::cout << "  - StartDate: " << msg.startDate << "\n";
        std::cout << "  - EndDate: " << msg.endDate << "\n";
        
        if (!send_message(sock, data.data(), data.size())) {
            std::cerr << "[TestClient] Error enviando mensaje\n";
            close(sock);
            return;
        }
        
        // Recibir respuesta
        std::vector<uint8_t> response(2048);
        ssize_t bytes = recv_message(sock, response.data(), response.size());
        
        if (bytes > 0) {
            response.resize(bytes);
            processSyncResponse(response);
        } else {
            std::cerr << "[TestClient] No se recibió respuesta\n";
        }
        
        close(sock);
    }

private:
    void processStorageResponse(const std::vector<uint8_t>& response) {
        if (response.empty()) return;
        
        uint8_t msgId = response[0];
        
        if (msgId == MSG_STORAGE_RESPONSE) {
            auto msg = StorageResponse::deserialize(response);
            std::cout << "\n✓ StorageResponse recibido:\n";
            std::cout << "  - StatusCode: " << (int)msg.statusCode;
            if (msg.statusCode == 0) {
                std::cout << " (Done - Guardado exitoso)\n";
            } else {
                std::cout << " (Error)\n";
            }
        } else if (msgId == MSG_STORAGE_ERROR) {
            auto msg = StorageError::deserialize(response);
            std::cout << "\n✗ StorageError recibido:\n";
            std::cout << "  - ErrorCode: " << msg.errorCode << " - ";
            switch (msg.errorCode) {
                case 401: std::cout << "Storage Full\n"; break;
                case 402: std::cout << "Write Failed\n"; break;
                case 403: std::cout << "Invalid Data\n"; break;
                default: std::cout << "Unknown Error\n"; break;
            }
        } else {
            std::cout << "\n? Respuesta desconocida (ID: " << (int)msgId << ")\n";
        }
    }
    
    void processSyncResponse(const std::vector<uint8_t>& response) {
        if (response.empty()) return;
        
        uint8_t msgId = response[0];
        
        if (msgId == MSG_STORAGE_SYNC_RESPONSE) {
            auto msg = StorageSyncResponse::deserialize(response);
            std::cout << "\n✓ StorageSyncResponse recibido:\n";
            std::cout << "  - Bloques recibidos: " << (int)msg.dataLength << "\n";
            
            for (size_t i = 0; i < msg.sensorDataBlocks.size(); i++) {
                const auto& block = msg.sensorDataBlocks[i];
                std::cout << "\n  Bloque " << (i + 1) << ":\n";
                std::cout << "    - SensorId: " << (int)block.sensorId << "\n";
                std::cout << "    - Date: " << block.date << "\n";
                std::cout << "    - Time: " << block.time << "\n";
                std::cout << "    - DataLength: " << (int)block.dataLength << "\n";
                std::cout << "    - Data (hex): ";
                for (auto byte : block.data) {
                    std::cout << std::hex << std::setw(2) << std::setfill('0') 
                             << (int)byte << " ";
                }
                std::cout << std::dec << "\n";
            }
        } else if (msgId == MSG_STORAGE_SYNC_ERROR) {
            auto msg = StorageSyncError::deserialize(response);
            std::cout << "\n✗ StorageSyncError recibido:\n";
            std::cout << "  - ErrorCode: " << msg.errorCode << " - ";
            if (msg.errorCode == 404) {
                std::cout << "Data Not Found\n";
            } else {
                std::cout << "Unknown Error\n";
            }
        } else {
            std::cout << "\n? Respuesta desconocida (ID: " << (int)msgId << ")\n";
        }
    }
};

int main() {
    std::cout << "===========================================\n";
    std::cout << "   PRUEBAS DEL NODO STORAGE\n";
    std::cout << "===========================================\n";
    std::cout << "Asegúrate de que nodeStorageMain esté corriendo\n";
    std::cout << "en el puerto 5003 antes de continuar.\n\n";
    
    std::cout << "Presiona Enter para comenzar las pruebas...";
    std::cin.get();
    
    StorageTestClient client;
    
    // Ejecutar tests
    client.testStorageSave();
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    client.testStorageSync();
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "\n===========================================\n";
    std::cout << "   PRUEBAS COMPLETADAS\n";
    std::cout << "===========================================\n";
    
    return 0;
}
