#pragma once
#include "node_base.h"
#include "messages.h"
#include "filesystem.h"
#include "../include/logger.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <sstream>

class NodeStorage : public NodeBase {
public:
    NodeStorage(int port, FileSystem* fileSystemInstance)
        : NodeBase(port), filesystem(fileSystemInstance), logger(fileSystemInstance, "sLogs.bin") {

        // Registrar handlers para cada tipo de mensaje
        dispatcher.registerHandler(MSG_STORAGE_SAVE,
                                   [this](const std::vector<uint8_t>& buf, int client_socket) {
                                       onStorageSave(buf, client_socket);
                                   });

        dispatcher.registerHandler(MSG_STORAGE_SYNC_REQUEST,
                                   [this](const std::vector<uint8_t>& buf, int client_socket) {
                                       onStorageSyncRequest(buf, client_socket);
                                   });
        dispatcher.registerHandler(MSG_LIST_SENSOR_REQUEST,
                                   [this](const std::vector<uint8_t>& buf, int client_socket) {
                                       onListSensorRequest(buf, client_socket);
                                   });

        dispatcher.registerHandler(MSG_DATA_REQUEST,
                                   [this](const std::vector<uint8_t>& buf, int client_socket) {
                                       onDataRequest(buf, client_socket);
                                   });
    }

    void onStorageSave(const std::vector<uint8_t>& buf, int client_socket) {
        auto msg = StorageSave::deserialize(buf);

        std::cout << "[StorageNode] Guardando datos - Sensor: " << (int)msg.sensorId
                  << ", Fecha: " << msg.date << ", Hora: " << msg.time
                  << ", DataLength: " << (int)msg.dataLength << "\n";

        bool success = saveToFilesystem(msg);

        if (success) {
            sendStorageResponse(client_socket, 0);  // 0 = Done
            logger.success("Datos guardados exitosamente - Sensor: " + std::to_string((int)msg.sensorId));
        } else {
            sendStorageError(client_socket, 402);  // 402 = Write failed
            logger.error("Error al guardar datos - Sensor: " + std::to_string((int)msg.sensorId));
        }

        close(client_socket);
    }

    void onStorageSyncRequest(const std::vector<uint8_t>& buf, int client_socket) {
        auto msg = StorageSyncRequest::deserialize(buf);

        std::string logMsg = "Solicitud de sincronización - Sensor: " + std::to_string((int)msg.sensorId) +
                             " | Rango: " + std::to_string(msg.startDate) + " - " + std::to_string(msg.endDate);
        logger.info(logMsg);

        // Recuperar datos del filesystem
        std::vector<SensorDataBlock> dataBlocks = retrieveDataFromFilesystem(msg);

        // Enviar respuesta
        if (!dataBlocks.empty()) {
            sendStorageSyncResponse(client_socket, dataBlocks);
            logger.success("Sincronización exitosa - " + std::to_string(dataBlocks.size()) + " bloques enviados");
        } else {
            sendStorageSyncError(client_socket, 404);  // 404 = Data not found
            logger.warning("No se encontraron datos para la sincronización solicitada");
        }

        close(client_socket);
    }

    void onListSensorRequest(const std::vector<uint8_t>& buf, int client_socket) {
        std::cout << "[StorageNode] ListSensorRequest recibido\n";

        std::vector<std::string> sensorIds = getAvailableSensors();

        // Crear respuesta
        ListSensorResponse response;
        response.message_id = MSG_LIST_SENSOR_RESPONSE;
        response.sensorCount = sensorIds.size();

        for (const auto& sensorId : sensorIds) {
            std::array<char, 16> id;
            std::memset(id.data(), 0, 16);
            std::memcpy(id.data(), sensorId.c_str(), std::min(sensorId.length(), size_t(16)));
            response.sensorIds.push_back(id);
        }

        auto data = response.serialize();
        send_message(client_socket, data.data(), data.size());

        std::cout << "[StorageNode] ListSensorResponse enviado (" << sensorIds.size() << " sensores)\n";
        close(client_socket);
    }

    // Handler para solicitud de datos de sensor
    void onDataRequest(const std::vector<uint8_t>& buf, int client_socket) {
        auto msg = DataRequestWithoutToken::deserialize(buf);

        std::string sensorId(msg.sensor_id, strnlen(msg.sensor_id, 16));
        std::string logMsg = "Solicitud de datos - Sensor: " + sensorId +
                             " | Rango: " + std::to_string(msg.startDate) + " - " + std::to_string(msg.endDate);
        logger.info(logMsg);

        // Mapear sensor_id string a uint8_t
        uint8_t sensorNumId = mapSensorIdToNumber(sensorId);

        if (sensorNumId == 0) {
            std::cerr << "[StorageNode] Sensor ID desconocido: " << sensorId << "\n";
            sendDataError(client_socket);
            close(client_socket);
            return;
        }

        // Buscar datos usando el mismo sistema que StorageSyncRequest
        std::vector<SensorDataBlock> blocks = retrieveDataByDateRange(sensorNumId, msg.startDate, msg.endDate);

        if (blocks.empty()) {
            logger.warning("No se encontraron datos para el sensor: " + sensorId);
            sendDataError(client_socket);
        } else {
            sendDataResponse(client_socket, sensorId, blocks);
        }

        close(client_socket);
    }

private:
    FileSystem* filesystem;
    Logger logger;

    // Mapeo de sensores conocidos
    std::map<std::string, uint8_t> sensorMap = {
        {"HC-SR04", 1},
        {"SW-420", 2},
        {"KY-038", 3},
        {"DHT11", 4}
    };

    // Mapeo inverso
    std::map<uint8_t, std::string> sensorMapReverse = {
        {1, "HC-SR04"},
        {2, "SW-420"},
        {3, "KY-038"},
        {4, "DHT11"}
    };

    uint8_t mapSensorIdToNumber(const std::string& sensorId) {
        auto it = sensorMap.find(sensorId);
        return (it != sensorMap.end()) ? it->second : 0;
    }

    std::string mapSensorNumberToId(uint8_t sensorNum) {
        auto it = sensorMapReverse.find(sensorNum);
        return (it != sensorMapReverse.end()) ? it->second : "";
    }

    std::vector<std::string> getAvailableSensors() {
        // Retornar lista de sensores conocidos
        // En un sistema real, escanearías los archivos del filesystem
        return {"HC-SR04", "SW-420", "KY-038", "DHT11"};
    }

    std::vector<SensorDataBlock> retrieveDataByDateRange(uint8_t sensorId, uint64_t startDate, uint64_t endDate) {
        std::vector<SensorDataBlock> results;

        // Buscar en todos los archivos del rango de fechas
        for (uint64_t date = startDate; date <= endDate; date++) {
            std::string dateStr = std::to_string(date);
            std::string mmdd;

            if (dateStr.length() >= 8) {
                mmdd = dateStr.substr(4, 4);
            } else if (dateStr.length() >= 4) {
                mmdd = dateStr.substr(dateStr.length() - 4);
            } else {
                continue;
            }

            std::string filename = "s" + std::to_string(sensorId) + mmdd + ".dat";
            std::vector<char> fileData = filesystem->readFile(filename);

            if (fileData.empty()) continue;

            // Parsear bloques del archivo
            size_t idx = 0;
            while (idx < fileData.size()) {
                if (idx + 1 + 8 + 8 + 1 > fileData.size()) break;

                SensorDataBlock block;
                std::memcpy(&block.sensorId, &fileData[idx], sizeof(uint8_t));
                idx += sizeof(uint8_t);

                std::memcpy(&block.date, &fileData[idx], sizeof(uint64_t));
                idx += sizeof(uint64_t);

                std::memcpy(&block.time, &fileData[idx], sizeof(uint64_t));
                idx += sizeof(uint64_t);

                std::memcpy(&block.dataLength, &fileData[idx], sizeof(uint8_t));
                idx += sizeof(uint8_t);

                if (idx + block.dataLength > fileData.size()) break;

                block.data.resize(block.dataLength);
                std::memcpy(block.data.data(), &fileData[idx], block.dataLength);
                idx += block.dataLength;

                results.push_back(block);
            }
        }

        std::cout << "[StorageNode] Recuperados " << results.size() << " registros\n";
        return results;
    }

    void sendDataResponse(int client_socket, const std::string& sensorId, const std::vector<SensorDataBlock>& blocks) {
        DataResponse response;
        response.message_id = MSG_DATA_RESPONSE;
        response.entriesCount = blocks.size();

        for (const auto& block : blocks) {
            SensorEntry entry;

            // sensor_id
            std::memset(entry.sensor_id, 0, 16);
            std::memcpy(entry.sensor_id, sensorId.c_str(), std::min(sensorId.length(), size_t(16)));

            entry.date = block.date;
            entry.time = block.time;

            // Desempaquetar value (2 bytes) y status (1 byte)
            if (block.data.size() >= 3) {
                uint16_t value = (static_cast<uint16_t>(block.data[0]) << 8) | block.data[1];
                entry.data_value = static_cast<float>(value);

                uint8_t statusByte = block.data[2];
                std::memset(entry.status, 0, 8);
                if (statusByte == 1) {
                    std::memcpy(entry.status, "ALERT", 5);
                } else {
                    std::memcpy(entry.status, "NORMAL", 6);
                }
            }

            response.entries.push_back(entry);
        }

        auto data = response.serialize();
        send_message(client_socket, data.data(), data.size());

        std::cout << "[StorageNode] DataResponse enviado (" << response.entriesCount << " entradas)\n";
    }

    void sendDataError(int client_socket) {
        // Enviar respuesta vacía
        DataResponse response;
        response.message_id = MSG_DATA_RESPONSE;
        response.entriesCount = 0;

        auto data = response.serialize();
        send_message(client_socket, data.data(), data.size());
    }

    // Guardar datos en el filesystem
    bool saveToFilesystem(const StorageSave& msg) {
        try {
            // s[sensorId][MMDD].dat
            std::string dateStr = std::to_string(msg.date);
            std::string mmdd;

            if (dateStr.length() >= 8) {
                // Extraer MMDD de YYYYMMDD (posiciones 4-7)
                mmdd = dateStr.substr(4, 4);
            } else if (dateStr.length() >= 4) {
                // Si es más corto, tomar los últimos 4 dígitos
                mmdd = dateStr.substr(dateStr.length() - 4);
            } else {
                // Padding si es muy corto
                mmdd = std::string(4 - dateStr.length(), '0') + dateStr;
            }

            std::string filename = "s" + std::to_string(msg.sensorId) + mmdd + ".dat";

            // Verificar longitud (debe ser <= 11)
            if (filename.length() > 11) {
                std::cerr << "[StorageNode] Error: Nombre demasiado largo: " << filename
                          << " (longitud: " << filename.length() << ")\n";
                return false;
            }

            std::cout << "[StorageNode] Usando filename: " << filename
                      << " (longitud: " << filename.length() << ")\n";

            // Verificar si el archivo existe
            std::vector<char> existing = filesystem->readFile(filename);
            if (existing.empty()) {
                std::cout << "[StorageNode] Creando nuevo archivo: " << filename << "\n";
                if (!filesystem->createFile(filename)) {
                    std::cerr << "[StorageNode] Error: No se pudo crear el archivo\n";
                    return false;
                }
            }

            // Serializar los datos para guardar
            std::stringstream ss;
            ss.write(reinterpret_cast<const char*>(&msg.sensorId), sizeof(uint8_t));
            ss.write(reinterpret_cast<const char*>(&msg.date), sizeof(uint64_t));
            ss.write(reinterpret_cast<const char*>(&msg.time), sizeof(uint64_t));
            ss.write(reinterpret_cast<const char*>(&msg.dataLength), sizeof(uint8_t));
            ss.write(reinterpret_cast<const char*>(msg.data.data()), msg.data.size());

            std::string dataStr = ss.str();
            filesystem->appendToFile(filename, dataStr);

            std::cout << "[StorageNode] Datos guardados exitosamente en " << filename
                      << " (" << dataStr.size() << " bytes)\n";
            return true;

        } catch (const std::exception& e) {
            std::cerr << "[StorageNode] Error al guardar: " << e.what() << "\n";
            return false;
        }
    }

    std::vector<SensorDataBlock> retrieveDataFromFilesystem(const StorageSyncRequest& msg) {
        std::vector<SensorDataBlock> results;

        try {
            // Extraer MMDD de la fecha de inicio
            std::string startDateStr = std::to_string(msg.startDate);
            std::string mmdd;

            if (startDateStr.length() >= 8) {
                mmdd = startDateStr.substr(4, 4);
            } else if (startDateStr.length() >= 4) {
                mmdd = startDateStr.substr(startDateStr.length() - 4);
            } else {
                mmdd = std::string(4 - startDateStr.length(), '0') + startDateStr;
            }

            std::string filename = "s" + std::to_string(msg.sensorId) + mmdd + ".dat";

            std::cout << "[StorageNode] Buscando archivo: " << filename << "\n";

            std::vector<char> fileData = filesystem->readFile(filename);

            if (fileData.empty()) {
                std::cout << "[StorageNode] Archivo no encontrado o vacío: " << filename << "\n";

                // Buscar en rango de fechas
                for (uint64_t date = msg.startDate; date <= msg.endDate; date++) {
                    std::string dateStr = std::to_string(date);
                    std::string altMmdd;

                    if (dateStr.length() >= 8) {
                        altMmdd = dateStr.substr(4, 4);
                    } else if (dateStr.length() >= 4) {
                        altMmdd = dateStr.substr(dateStr.length() - 4);
                    } else {
                        altMmdd = std::string(4 - dateStr.length(), '0') + dateStr;
                    }

                    std::string altFilename = "s" + std::to_string(msg.sensorId)
                                              + altMmdd + ".dat";

                    std::vector<char> altData = filesystem->readFile(altFilename);
                    if (!altData.empty()) {
                        std::cout << "[StorageNode] Encontrado: " << altFilename << "\n";
                        fileData = altData;
                        break;
                    }
                }

                if (fileData.empty()) {
                    return results;
                }
            }

            // Parsear bloques
            size_t idx = 0;
            while (idx < fileData.size()) {
                if (idx + 1 + 8 + 8 + 1 > fileData.size()) {
                    break;
                }

                SensorDataBlock block;
                std::memcpy(&block.sensorId, &fileData[idx], sizeof(uint8_t));
                idx += sizeof(uint8_t);

                std::memcpy(&block.date, &fileData[idx], sizeof(uint64_t));
                idx += sizeof(uint64_t);

                std::memcpy(&block.time, &fileData[idx], sizeof(uint64_t));
                idx += sizeof(uint64_t);

                std::memcpy(&block.dataLength, &fileData[idx], sizeof(uint8_t));
                idx += sizeof(uint8_t);

                if (idx + block.dataLength > fileData.size()) {
                    std::cerr << "[StorageNode] Datos corruptos en archivo\n";
                    break;
                }

                block.data.resize(block.dataLength);
                std::memcpy(block.data.data(), &fileData[idx], block.dataLength);
                idx += block.dataLength;

                results.push_back(block);
            }

            std::cout << "[StorageNode] Recuperados " << results.size()
                      << " bloques de datos\n";

        } catch (const std::exception& e) {
            std::cerr << "[StorageNode] Error al recuperar datos: " << e.what() << "\n";
        }

        return results;
    }

    // Enviar respuesta exitosa de guardado
    void sendStorageResponse(int client_socket, uint8_t statusCode) {
        StorageResponse response;
        response.message_id = MSG_STORAGE_RESPONSE;
        response.statusCode = statusCode;

        auto data = response.serialize();
        send_message(client_socket, data.data(), data.size());

        std::cout << "[StorageNode] StorageResponse enviado (status=" << (int)statusCode << ")\n";
    }

    // Enviar error de guardado
    void sendStorageError(int client_socket, uint16_t errorCode) {
        StorageError error;
        error.message_id = MSG_STORAGE_ERROR;
        error.errorCode = errorCode;

        auto data = error.serialize();
        send_message(client_socket, data.data(), data.size());

        std::cout << "[StorageNode] StorageError enviado (error=" << errorCode << ")\n";
    }

    // Enviar respuesta de sincronización
    void sendStorageSyncResponse(int client_socket, const std::vector<SensorDataBlock>& blocks) {
        StorageSyncResponse response;
        response.message_id = MSG_STORAGE_SYNC_RESPONSE;
        response.dataLength = blocks.size();
        response.sensorDataBlocks = blocks;

        auto data = response.serialize();
        send_message(client_socket, data.data(), data.size());

        std::cout << "[StorageNode] StorageSyncResponse enviado (" << blocks.size() << " bloques)\n";
    }

    // Enviar error de sincronización
    void sendStorageSyncError(int client_socket, uint16_t errorCode) {
        StorageSyncError error;
        error.message_id = MSG_STORAGE_SYNC_ERROR;
        error.errorCode = errorCode;

        auto data = error.serialize();
        send_message(client_socket, data.data(), data.size());

        std::cout << "[StorageNode] StorageSyncError enviado (error=" << errorCode << ")\n";
    }
};
