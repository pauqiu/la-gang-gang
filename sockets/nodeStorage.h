#pragma once
#include "node_base.h"
#include "messages.h"
#include "filesystem.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <sstream>

class NodeStorage : public NodeBase {
public:
    NodeStorage(int port, FileSystem* fileSystemInstance)
        : NodeBase(port), filesystem(fileSystemInstance) {

        // Registrar handlers para cada tipo de mensaje
        dispatcher.registerHandler(MSG_STORAGE_SAVE,
                                   [this](const std::vector<uint8_t>& buf, int client_socket) {
                                       onStorageSave(buf, client_socket);
                                   });

        dispatcher.registerHandler(MSG_STORAGE_SYNC_REQUEST,
                                   [this](const std::vector<uint8_t>& buf, int client_socket) {
                                       onStorageSyncRequest(buf, client_socket);
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
        } else {
            sendStorageError(client_socket, 402);  // 402 = Write failed
        }

        close(client_socket);
    }

    void onStorageSyncRequest(const std::vector<uint8_t>& buf, int client_socket) {
        auto msg = StorageSyncRequest::deserialize(buf);

        std::cout << "[StorageNode] Solicitud de sincronización - Sensor: " << (int)msg.sensorId
                  << ", StartDate: " << msg.startDate << ", EndDate: " << msg.endDate << "\n";

        // Recuperar datos del filesystem
        std::vector<SensorDataBlock> dataBlocks = retrieveDataFromFilesystem(msg);

        // Enviar respuesta
        if (!dataBlocks.empty()) {
            sendStorageSyncResponse(client_socket, dataBlocks);
        } else {
            sendStorageSyncError(client_socket, 404);  // 404 = Data not found
        }

        close(client_socket);
    }

private:
    FileSystem* filesystem;

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
22
        auto data = error.serialize();
        send_message(client_socket, data.data(), data.size());

        std::cout << "[StorageNode] StorageSyncError enviado (error=" << errorCode << ")\n";
    }
};
