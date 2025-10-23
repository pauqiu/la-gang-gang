#include "nodeClient.h"
#include "messages.h"
#include "communication.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <vector>
#include <map>

class StorageTestClient {
public:
    // Mapeo de sensor_id string a uint8_t
    std::map<std::string, uint8_t> sensorMap = {
        {"PIR001", 1},
        {"DHT11A", 2},
        {"HC001", 3},
        {"VB001", 4}
    };

    // Test 1: Guardar datos reales de los sensores
    void testStorageSaveReal() {
        std::cout << "\n========================================\n";
        std::cout << "TEST 1: Guardar datos reales de sensores\n";
        std::cout << "========================================\n";

        // Datos del CSV organizados por sensor
        struct Reading {
            std::string timestamp;  // "2025-09-25 22:55:01"
            uint16_t value;         // data_value
            std::string status;     // NORMAL/ALERT
        };

        std::map<std::string, std::vector<Reading>> sensorData = {
            {"PIR001", {
                           {"2025-09-25 22:55:01", 0, "NORMAL"},
                           {"2025-09-25 22:56:04", 0, "NORMAL"},
                           {"2025-09-25 22:57:02", 1, "ALERT"},
                           {"2025-09-25 22:57:59", 0, "NORMAL"},
                           {"2025-09-25 22:58:20", 0, "NORMAL"}
                       }},
            {"DHT11A", {
                           {"2025-09-25 22:54:55", 24, "NORMAL"},
                           {"2025-09-25 22:55:55", 25, "NORMAL"},
                           {"2025-09-25 22:56:55", 42, "ALERT"},
                           {"2025-09-25 22:57:55", 20, "NORMAL"},
                           {"2025-09-25 22:58:55", 23, "NORMAL"}
                       }},
            {"HC001", {
                          {"2025-09-25 22:54:12", 110, "NORMAL"},
                          {"2025-09-25 22:55:12", 325, "NORMAL"},
                          {"2025-09-25 22:56:15", 33, "NORMAL"},
                          {"2025-09-25 22:57:13", 7, "ALERT"},
                          {"2025-09-25 22:58:21", 23, "NORMAL"}
                      }},
            {"VB001", {
                          {"2025-09-25 22:54:50", 0, "NORMAL"},
                          {"2025-09-25 22:55:50", 0, "NORMAL"},
                          {"2025-09-25 22:56:51", 0, "NORMAL"},
                          {"2025-09-25 22:57:51", 1, "ALERT"},
                          {"2025-09-25 22:58:53", 1, "ALERT"}
                      }}
        };

        // Procesar cada sensor
        for (const auto& [sensorName, readings] : sensorData) {
            uint8_t sensorId = sensorMap[sensorName];

            std::cout << "\n--- Procesando sensor " << sensorName
                      << " (ID: " << (int)sensorId << ") ---\n";

            for (const auto& reading : readings) {
                int sock = connect_to("127.0.0.1", 5004);
                if (sock < 0) {
                    std::cerr << "[TestClient] Error conectando a Storage\n";
                    continue;
                }

                // Parsear timestamp "2025-09-25 22:55:01"
                auto [date, time] = parseTimestamp(reading.timestamp);

                // Empaquetar datos: [value(2bytes)][status(1byte)]
                std::vector<uint8_t> data;
                data.push_back((reading.value >> 8) & 0xFF);
                data.push_back(reading.value & 0xFF);
                data.push_back(reading.status == "ALERT" ? 1 : 0);

                StorageSave msg;
                msg.sensorId = sensorId;
                msg.date = date;
                msg.time = time;
                msg.data = data;
                msg.dataLength = data.size();

                auto serialized = msg.serialize();

                std::cout << "[TestClient] Enviando: " << reading.timestamp
                          << " | Value: " << reading.value
                          << " | Status: " << reading.status << "\n";

                if (!send_message(sock, serialized.data(), serialized.size())) {
                    std::cerr << "[TestClient] Error enviando mensaje\n";
                    close(sock);
                    continue;
                }

                // Recibir respuesta
                std::vector<uint8_t> response(1024);
                ssize_t bytes = recv_message(sock, response.data(), response.size());
                if (bytes > 0) {
                    response.resize(bytes);
                    processStorageResponse(response);
                }

                close(sock);
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
    }

    // Test 2: Sincronizar datos
    void testStorageSync() {
        std::cout << "\n========================================\n";
        std::cout << "TEST 2: Sincronizar datos de sensores\n";
        std::cout << "========================================\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Sincronizar cada sensor
        for (const auto& [sensorName, sensorId] : sensorMap) {
            int sock = connect_to("127.0.0.1", 5004);
            if (sock < 0) {
                std::cerr << "[TestClient] Error conectando a Storage\n";
                continue;
            }

            StorageSyncRequest msg;
            msg.sensorId = sensorId;
            msg.startDate = 20250924;  // 2025-09-24
            msg.endDate = 20250926;    // 2025-09-26

            auto data = msg.serialize();

            std::cout << "\n[TestClient] Sincronizando " << sensorName
                      << " (ID: " << (int)sensorId << ")\n";

            if (!send_message(sock, data.data(), data.size())) {
                std::cerr << "[TestClient] Error enviando mensaje\n";
                close(sock);
                continue;
            }

            std::vector<uint8_t> response(8192);  // Buffer más grande
            ssize_t bytes = recv_message(sock, response.data(), response.size());

            if (bytes > 0) {
                response.resize(bytes);
                processSyncResponse(response, sensorName);
            }

            close(sock);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }

private:
    // Parsear timestamp "2025-09-25 22:55:01" → (date: 20250925, time: 225501)
    std::pair<uint64_t, uint64_t> parseTimestamp(const std::string& timestamp) {
        // Formato: "YYYY-MM-DD HH:MM:SS"
        int year, month, day, hour, minute, second;
        sscanf(timestamp.c_str(), "%d-%d-%d %d:%d:%d",
               &year, &month, &day, &hour, &minute, &second);

        uint64_t date = year * 10000 + month * 100 + day;  // YYYYMMDD
        uint64_t time = hour * 10000 + minute * 100 + second;  // HHMMSS

        return {date, time};
    }

    void processStorageResponse(const std::vector<uint8_t>& response) {
        if (response.empty()) return;

        uint8_t msgId = response[0];

        if (msgId == MSG_STORAGE_RESPONSE) {
            auto msg = StorageResponse::deserialize(response);
            std::cout << "  ✓ Guardado " << (msg.statusCode == 0 ? "exitoso" : "fallido") << "\n";
        } else if (msgId == MSG_STORAGE_ERROR) {
            auto msg = StorageError::deserialize(response);
            std::cout << "  ✗ Error: " << msg.errorCode << "\n";
        }
    }

    void processSyncResponse(const std::vector<uint8_t>& response,
                             const std::string& sensorName) {
        if (response.empty()) return;

        uint8_t msgId = response[0];

        if (msgId == MSG_STORAGE_SYNC_RESPONSE) {
            auto msg = StorageSyncResponse::deserialize(response);
            std::cout << "\nRecuperados " << (int)msg.dataLength << " registros de "
                      << sensorName << ":\n";
            std::cout << std::string(60, '-') << "\n";

            for (size_t i = 0; i < msg.sensorDataBlocks.size(); i++) {
                const auto& block = msg.sensorDataBlocks[i];

                // Desempaquetar datos: [value(2bytes)][status(1byte)]
                if (block.data.size() < 3) {
                    std::cout << "    [" << (i+1) << "] Error: datos incompletos\n";
                    continue;
                }

                // Reconstruir el valor de 16 bits desde 2 bytes
                uint16_t value = (static_cast<uint16_t>(block.data[0]) << 8) | block.data[1];
                uint8_t statusByte = block.data[2];
                std::string status = (statusByte == 1) ? "ALERT" : "NORMAL";

                // Convertir date y time a formato legible
                std::string dateStr = std::to_string(block.date);  // YYYYMMDD
                std::string timeStr = std::to_string(block.time);  // HHMMSS

                // Formatear con padding de ceros si es necesario
                while (timeStr.length() < 6) timeStr = "0" + timeStr;

                std::string year = dateStr.substr(0, 4);
                std::string month = dateStr.substr(4, 2);
                std::string day = dateStr.substr(6, 2);

                std::string hour = timeStr.substr(0, 2);
                std::string minute = timeStr.substr(2, 2);
                std::string second = timeStr.substr(4, 2);

                // Mostrar en formato legible
                std::cout << "    [" << std::setw(2) << (i+1) << "] "
                          << year << "-" << month << "-" << day << " "
                          << hour << ":" << minute << ":" << second << " | "
                          << "Value: " << std::setw(4) << value << " | "
                          << "Status: " << std::setw(6) << std::left << status
                          << std::right << "\n";
            }
            std::cout << std::string(60, '-') << "\n";

        } else if (msgId == MSG_STORAGE_SYNC_ERROR) {
            auto msg = StorageSyncError::deserialize(response);
            std::cout << "  Error de sincronización: " << msg.errorCode << "\n";
        } else {
            std::cout << "  Respuesta desconocida (ID: " << (int)msgId << ")\n";
        }
    }

};

int main() {
    std::cout << "===========================================\n";
    std::cout << "   PRUEBAS CON DATOS REALES DE SENSORES\n";
    std::cout << "===========================================\n";
    std::cout << "Asegúrate de que nodeStorageMain esté corriendo\n";
    std::cout << "en el puerto 5004.\n\n";

    std::cout << "Presiona Enter para comenzar...";
    std::cin.get();

    StorageTestClient client;

    // Guardar datos reales
    client.testStorageSaveReal();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Sincronizar y mostrar datos
    client.testStorageSync();

    std::cout << "\n===========================================\n";
    std::cout << "   PRUEBAS COMPLETADAS\n";
    std::cout << "===========================================\n";

    return 0;
}
