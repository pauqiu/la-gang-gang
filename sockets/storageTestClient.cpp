#include "nodeClient.h"
#include "messages.h"
#include "communication.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <vector>

class StorageTestClient {
public:
    // Test 1: Guardar datos (para varios sensores)
    void testStorageSaveMultiple() {
        std::cout << "\n========================================\n";
        std::cout << "TEST 1: Guardar datos de múltiples sensores\n";
        std::cout << "========================================\n";

        // Datos de ejemplo para dos sensores (1 y 2)
        struct SensorData {
            uint8_t id;
            std::vector<std::tuple<int, int, std::vector<uint8_t>>> registros;
        };

        std::vector<SensorData> sensores = {
            {1, {
                    {20241021, 143000, {0x01, 0x02, 0x03, 0x04}},
                    {20241021, 150000, {0x05, 0x06, 0x07, 0x08}},
                    {20241021, 153000, {0x09, 0x0A, 0x0B, 0x0C}}
                }},
            {2, {
                    {20241021, 143500, {0x11, 0x12, 0x13, 0x14}},
                    {20241021, 150500, {0x15, 0x16, 0x17, 0x18}},
                    {20241021, 153500, {0x19, 0x1A, 0x1B, 0x1C}}
                }}
        };

        // Enviar los registros de todos los sensores
        for (const auto& sensor : sensores) {
            for (const auto& registro : sensor.registros) {
                int sock = connect_to("127.0.0.1", 5004);
                if (sock < 0) {
                    std::cerr << "[TestClient] Error conectando a Storage\n";
                    continue;
                }

                StorageSave msg;
                msg.sensorId = sensor.id;
                msg.date = std::get<0>(registro);
                msg.time = std::get<1>(registro);
                msg.data = std::get<2>(registro);
                msg.dataLength = msg.data.size();

                auto data = msg.serialize();

                std::cout << "\n[TestClient] Enviando StorageSave:\n";
                std::cout << "  - SensorId: " << (int)msg.sensorId << "\n";
                std::cout << "  - Date: " << msg.date << "\n";
                std::cout << "  - Time: " << msg.time << "\n";
                std::cout << "  - DataLength: " << (int)msg.dataLength << "\n";

                if (!send_message(sock, data.data(), data.size())) {
                    std::cerr << "[TestClient] Error enviando mensaje\n";
                    close(sock);
                    continue;
                }

                // Esperar respuesta
                std::vector<uint8_t> response(1024);
                ssize_t bytes = recv_message(sock, response.data(), response.size());
                if (bytes > 0) {
                    response.resize(bytes);
                    processStorageResponse(response);
                } else {
                    std::cerr << "[TestClient] No se recibió respuesta\n";
                }

                close(sock);

                // Pequeña pausa entre mensajes
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
        }

    }

    // Test 2: Sincronizar datos (para ambos sensores)
    void testStorageSync() {
        std::cout << "\n========================================\n";
        std::cout << "TEST 2: Sincronizar datos de sensores\n";
        std::cout << "========================================\n";

        // Esperar un poco para que se guarden los datos
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Probar sincronización de ambos sensores
        for (int sensorId = 1; sensorId <= 2; ++sensorId) {
            int sock = connect_to("127.0.0.1", 5004);
            if (sock < 0) {
                std::cerr << "[TestClient] Error conectando a Storage\n";
                return;
            }

            StorageSyncRequest msg;
            msg.sensorId = sensorId;
            msg.startDate = 20241020;
            msg.endDate = 20241022;

            auto data = msg.serialize();

            std::cout << "\n[TestClient] Enviando StorageSyncRequest:\n";
            std::cout << "  - SensorId: " << (int)msg.sensorId << "\n";
            std::cout << "  - StartDate: " << msg.startDate << "\n";
            std::cout << "  - EndDate: " << msg.endDate << "\n";

            if (!send_message(sock, data.data(), data.size())) {
                std::cerr << "[TestClient] Error enviando mensaje\n";
                close(sock);
                return;
            }

            // Recibir respuesta
            std::vector<uint8_t> response(4096);
            ssize_t bytes = recv_message(sock, response.data(), response.size());

            if (bytes > 0) {
                response.resize(bytes);
                processSyncResponse(response);
            } else {
                std::cerr << "[TestClient] No se recibió respuesta\n";
            }

            close(sock);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }

private:
    void processStorageResponse(const std::vector<uint8_t>& response) {
        if (response.empty()) return;

        uint8_t msgId = response[0];

        if (msgId == MSG_STORAGE_RESPONSE) {
            auto msg = StorageResponse::deserialize(response);
            std::cout << "\n✓ StorageResponse recibido:\n";
            std::cout << "  - StatusCode: " << (int)msg.statusCode;
            if (msg.statusCode == 0)
                std::cout << " (Guardado exitoso)\n";
            else
                std::cout << " (Error)\n";
        } else if (msgId == MSG_STORAGE_ERROR) {
            auto msg = StorageError::deserialize(response);
            std::cout << "\n✗ StorageError recibido:\n";
            std::cout << "  - ErrorCode: " << msg.errorCode << "\n";
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
                for (auto byte : block.data)
                    std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
                std::cout << std::dec << "\n";
            }
        } else if (msgId == MSG_STORAGE_SYNC_ERROR) {
            auto msg = StorageSyncError::deserialize(response);
            std::cout << "\n✗ StorageSyncError recibido:\n";
            std::cout << "  - ErrorCode: " << msg.errorCode << "\n";
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
    std::cout << "en el puerto 5004 antes de continuar.\n\n";

    std::cout << "Presiona Enter para comenzar las pruebas...";
    std::cin.get();

    StorageTestClient client;

    // Enviar múltiples datos de sensores
    client.testStorageSaveMultiple();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Sincronizar datos de ambos sensores
    client.testStorageSync();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "\n===========================================\n";
    std::cout << "   PRUEBAS COMPLETADAS\n";
    std::cout << "===========================================\n";

    return 0;
}
