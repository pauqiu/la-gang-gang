#include "nodeClient.h"
#include "messages.h"
#include "communication.h"
#include "sensorRegistry.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <vector>
#include <map>

class EndToEndTest {
public:

    // Paso 1: Inyectar datos de prueba en el storage
    void injectTestData() {
        std::cout << "1: Inyectando datos de prueba\n";

        // Datos de prueba para cada sensor
        struct Reading {
            std::string timestamp;
            uint16_t value;
            std::string status;
        };

        std::map<std::string, std::vector<Reading>> testData = {
            {"PIR001", {
                {"2025-09-25 10:00:00", 0, "NORMAL"},
                {"2025-09-25 10:05:00", 1, "ALERT"},
                {"2025-09-25 10:10:00", 0, "NORMAL"}
            }},
            {"DHT11A", {
                {"2025-09-25 10:00:00", 22, "NORMAL"},
                {"2025-09-25 10:05:00", 24, "NORMAL"},
                {"2025-09-25 10:10:00", 45, "ALERT"},
                {"2025-09-25 10:15:00", 23, "NORMAL"}
            }},
            {"HC001", {
                {"2025-09-25 10:00:00", 150, "NORMAL"},
                {"2025-09-25 10:05:00", 200, "NORMAL"}
            }}
        };

        for (const auto& [sensorName, readings] : testData) {
            auto sensorIdOpt = SensorRegistry::getInstance().getNumericId(sensorName);
            if (!sensorIdOpt.has_value()) {
                std::cerr << "[Test] Sensor desconocido: " << sensorName << "\n";
                continue;
            }
            uint8_t sensorId = sensorIdOpt.value();
            
            std::cout << "2: Inyectando datos para " << sensorName << "\n";
            
            for (const auto& reading : readings) {
                int sock = connect_to("127.0.0.1", 5003);  // Puerto del storage
                if (sock < 0) {
                    std::cerr << "[Test] Error conectando a Storage\n";
                    continue;
                }

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

                std::cout << "  Guardando: " << reading.timestamp 
                          << " | Value: " << reading.value 
                          << " | Status: " << reading.status << "\n";

                if (!send_message(sock, serialized.data(), serialized.size())) {
                    std::cerr << "[Test] Error enviando mensaje\n";
                    close(sock);
                    continue;
                }

                // Recibir confirmación
                std::vector<uint8_t> response(1024);
                ssize_t bytes = recv_message(sock, response.data(), response.size());
                if (bytes > 0) {
                    response.resize(bytes);
                    if (response[0] == MSG_STORAGE_RESPONSE) {
                        std::cout << "  Guardado exitoso\n";
                    }
                }

                close(sock);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        std::cout << "\nDatos inyectados correctamente\n";
    }

    // Paso 2: Autenticar cliente
    void authenticateClient(NodeClient& client) {
        std::cout << "\n========================================\n";
        std::cout << "PASO 2: Autenticando cliente\n";
        std::cout << "========================================\n";

        // Enviar autenticación al auth server
        bool authSent = client.sendAuthentication("admin01", "sysadmin_01");
        if (!authSent) {
            std::cerr << "Error enviando autenticación\n";
            return;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Validar sesión con proxy
        bool validated = client.validateSessionWithProxy();
        if (validated) {
            std::cout << "Cliente autenticado y validado correctamente\n";
        } else {
            std::cerr << "Error en validación de sesión\n";
        }
    }

    // Paso 3: Solicitar lista de sensores
    void testListSensors(NodeClient& client) {
        std::cout << "\n========================================\n";
        std::cout << "PASO 3: Solicitando lista de sensores\n";
        std::cout << "========================================\n";

        auto sensors = client.requestSensorList();
        
        if (sensors.empty()) {
            std::cout << "No se recibieron sensores\n";
        } else {
            std::cout << "\nLista de sensores recibida:\n";
            for (const auto& sensor : sensors) {
                std::cout << "  - " << sensor << "\n";
            }
        }
    }

    // Paso 4: Solicitar datos de un sensor específico
    void testRequestSensorData(NodeClient& client, const std::string& sensorId,
                               uint64_t startDate, uint64_t endDate) {
        std::cout << "\n========================================\n";
        std::cout << "PASO 4: Solicitando datos de " << sensorId << "\n";
        std::cout << "========================================\n";
        std::cout << "Rango: " << startDate << " - " << endDate << "\n";

        auto entries = client.requestSensorData(sensorId, startDate, endDate);
        
        if (entries.empty()) {
            std::cout << "No se recibieron datos para " << sensorId << "\n";
        } else {
            std::cout << "\nDatos recibidos (" << entries.size() << " entradas):\n";
            //printSensorData(entries);
        }
    }

private:
    // Parsear timestamp "YYYY-MM-DD HH:MM:SS" → (date, time)
    std::pair<uint64_t, uint64_t> parseTimestamp(const std::string& timestamp) {
        int year, month, day, hour, minute, second;
        sscanf(timestamp.c_str(), "%d-%d-%d %d:%d:%d",
               &year, &month, &day, &hour, &minute, &second);

        uint64_t date = year * 10000 + month * 100 + day;
        uint64_t time = hour * 10000 + minute * 100 + second;

        return {date, time};
    }

    // Imprimir datos de sensores en formato tabla
    void printSensorData(const std::vector<SensorEntry>& entries) {
        std::cout << std::string(80, '=') << "\n";
        std::cout << std::left << std::setfill(' ')
                  << std::setw(20) << "Timestamp"
                  << std::setw(15) << "Sensor"
                  << std::setw(15) << "Value"
                  << std::setw(10) << "Status" << "\n";
        std::cout << std::string(80, '-') << "\n";

        for (const auto& entry : entries) {
            // Formatear fecha y hora
            std::string dateStr = std::to_string(entry.date);
            std::string timeStr = std::to_string(entry.time);
            
            while (timeStr.length() < 6) timeStr = "0" + timeStr;
            
            std::string timestamp = dateStr.substr(0, 4) + "-" + 
                                   dateStr.substr(4, 2) + "-" + 
                                   dateStr.substr(6, 2) + " " +
                                   timeStr.substr(0, 2) + ":" + 
                                   timeStr.substr(2, 2) + ":" + 
                                   timeStr.substr(4, 2);

            std::string sensorId(entry.sensor_id, strnlen(entry.sensor_id, 16));
            std::string status(entry.status, strnlen(entry.status, 8));

            std::cout << std::left << std::setfill(' ')
                      << std::setw(20) << timestamp
                      << std::setw(15) << sensorId
                      << std::setw(15) << entry.data_value
                      << std::setw(10) << status << "\n";
        }
        
        std::cout << std::string(80, '=') << "\n";
    }
};

int main() {
    std::cout << "\n";
    std::cout << "TEST END-TO-END: FLUJO COMPLETOn";
    std::cout << "  - nodeAuthMain (puerto 5001)\n";
    std::cout << "  - nodeProxyMain (puerto 5002)\n";
    std::cout << "  - nodeStorageMain (puerto 5003)\n\n";

    std::cout << "Presiona Enter para comenzar...";
    std::cin.get();

    EndToEndTest test;
    
    // Paso 1: Inyectar datos de prueba
    test.injectTestData();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Crear cliente
    NodeClient client;
    
    // Paso 2: Autenticar
    test.authenticateClient(client);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Paso 3: Solicitar lista de sensores
    test.testListSensors(client);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Paso 4: Solicitar datos de sensores específicos
    test.testRequestSensorData(client, "PIR001", 20250925, 20250925);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    test.testRequestSensorData(client, "DHT11A", 20250925, 20250925);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    test.testRequestSensorData(client, "HC001", 20250925, 20250926);
    
    std::cout << "\n";
    std::cout << "TEST COMPLETADO\n";

    return 0;
}
