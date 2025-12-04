#pragma once
#include "node_base.h"
#include "messages.h"
#include "filesystem.h"
#include "../include/logger.h"
#include "endpoints.h"
#include "health_checker.h"
#include <iostream>
#include <map>
#include <cstring>
#include <array>
#include <sstream>
#include <algorithm>

struct StorageEndpoint {
    std::string ip;
    int port;
};

class NodeProxy : public NodeBase {
public:
    NodeProxy(int port, FileSystem* fs) : NodeBase(port), logger(fs, "pLogs.txt"), roundRobinIndex(0) {
        // Configurar storages disponibles
        storages.push_back({getStorageIp(), getStoragePort()});
        if (hasStorage2()) {
            storages.push_back({getStorage2Ip(), getStorage2Port()});
            logger.info("Round-Robin configurado con 2 storages");
        } else {
            logger.info("Un solo storage configurado");
        }
        // Configurar soporte de logs usando método de clase base
        setupLogSupport(fs, "pLogs.txt", NODE_PROXY);
        
        dispatcher.registerHandler(MSG_TOKEN_REGISTER, 
            [this](const std::vector<uint8_t>& buf, int client_socket) { 
                onTokenRegister(buf, client_socket); 
            });
        
        dispatcher.registerHandler(MSG_SESSION_VALIDATE, 
            [this](const std::vector<uint8_t>& buf, int client_socket) { 
                onSessionValidate(buf, client_socket); 
            });
        
        dispatcher.registerHandler(MSG_LIST_SENSOR_REQUEST,
            [this](const std::vector<uint8_t>& buf, int client_socket) {
                onListSensorRequest(buf, client_socket);
            });
        
        dispatcher.registerHandler(MSG_DATA_REQUEST,
            [this](const std::vector<uint8_t>& buf, int client_socket) {
                onDataRequest(buf, client_socket);
            });

        dispatcher.registerHandler(MSG_LOG_REQUEST,
            [this](const std::vector<uint8_t>& buf, int client_socket) {
                onLogRequest(buf, client_socket);
            });
    }

private:
    Logger logger;
    std::vector<StorageEndpoint> storages;
    size_t roundRobinIndex;
    
    // Selecciona el siguiente storage activo (round-robin)
    StorageEndpoint* selectActiveStorage() {
        if (storages.empty()) return nullptr;
        
        size_t attempts = 0;
        while (attempts < storages.size()) {
            size_t idx = roundRobinIndex % storages.size();
            roundRobinIndex++;
            
            auto& storage = storages[idx];
            if (HealthChecker::isNodeAlive(storage.ip, storage.port)) {
                logger.info("Storage seleccionado: " + storage.ip + ":" + std::to_string(storage.port));
                return &storage;
            }
            logger.warning("Storage " + storage.ip + ":" + std::to_string(storage.port) + " no disponible");
            attempts++;
        }
        
        logger.error("Ningún storage disponible");
        return nullptr;
    }
    
    // TODO: Integrar con filesystem para persistir tokens
    // Los tokens deben guardarse en archivo y leerse desde ahí
    // Estructura temporal en memoria (se pierde al reiniciar)
    struct TokenData {
        std::array<uint8_t, 32> token;
        uint8_t role;
    };
    std::map<std::string, TokenData> validTokens; // key: username

    // Registrar token recibido de auth
    void onTokenRegister(const std::vector<uint8_t>& buf, int client_socket) {
        auto msg = TokenNotif::deserialize(buf);
        
        // Extraer username
        std::string username(msg.username, strnlen(msg.username, 16));
        
        // Almacenar token válido asociado al username
        // TODO: Guardar en filesystem en lugar de memoria
        TokenData data;
        std::memcpy(data.token.data(), msg.token, 32);
        data.role = msg.role;
        validTokens[username] = data;
        
        std::string tokenHex = tokenToString(msg.token);
        logger.info("TokenNotif recibido - User: " + username + 
                    " | Role: " + std::to_string((int)msg.role) + 
                    " | Token: " + tokenHex.substr(0, 16) + "...");
        
        close(client_socket);
    }

    // Validar sesión recibida de client
    void onSessionValidate(const std::vector<uint8_t>& buf, int client_socket) {
        auto msg = SessionValidate::deserialize(buf);
        
        // Extraer username
        std::string username(msg.username, strnlen(msg.username, 16));
        std::string tokenHex = tokenToString(msg.token);
        
        logger.info("Validando sesión - User: " + username + 
                    " | Token: " + tokenHex.substr(0, 16) + "...");
        
        // TODO: Buscar token en filesystem en lugar de memoria
        if (isTokenValid(username, msg.token)) {
            sendSessionOk(client_socket, validTokens[username].role);
            logger.success("Sesión validada exitosamente - User: " + username);
        } else {
            sendSessionError(client_socket, 3);
            logger.warning("Sesión inválida - User: " + username);
        }
        
        close(client_socket);
    }

    bool isTokenValid(const std::string& username, const uint8_t token[32]) {
        // TODO: Verificar en filesystem
        // Debe buscar el usuario y comparar el token almacenado con el recibido
        
        // Verificar si el usuario existe
        auto it = validTokens.find(username);
        if (it == validTokens.end()) {
            return false;
        }
        
        // Comparar tokens
        return std::memcmp(it->second.token.data(), token, 32) == 0;
    }

    void sendSessionOk(int client_socket, uint8_t role) {
        SessionOk response;
        response.message_id = MSG_SESSION_OK;
        response.role = role;
        
        auto data = response.serialize();
        send_message(client_socket, data.data(), data.size());
        logger.info("SessionOk enviado (role=" + std::to_string((int)role) + ")");
    }

    void sendSessionError(int client_socket, uint8_t errorCode) {
        SessionError error;
        error.message_id = MSG_SESSION_ERROR;
        error.error_code = errorCode;
        
        auto data = error.serialize();
        send_message(client_socket, data.data(), data.size());
        logger.error("SessionError enviado (error_code=" + std::to_string((int)errorCode) + ")");
    }

    std::string tokenToString(const uint8_t token[32]) {
        std::string result;
        result.reserve(64);
        for (int i = 0; i < 32; i++) {
            char buf[3];
            snprintf(buf, sizeof(buf), "%02x", token[i]);
            result += buf;
        }
        return result;
    }
    
    // Validar token sin username (busca en todos los tokens)
    bool validateToken(const uint8_t token[32], std::string& outUsername) {
        for (const auto& pair : validTokens) {
            if (std::memcmp(pair.second.token.data(), token, 32) == 0) {
                outUsername = pair.first;
                return true;
            }
        }
        return false;
    }
    
    // Handler para solicitud de lista de sensores
    void onListSensorRequest(const std::vector<uint8_t>& buf, int client_socket) {
        // 1. Deserializar mensaje del cliente
        auto clientMsg = ListSensorRequest::deserialize(buf);
        
        logger.info("ListSensorRequest recibido");
        
        // 2. Validar token
        std::string username;
        if (!validateToken(clientMsg.token, username)) {
            logger.warning("Token inválido, rechazando solicitud de lista de sensores");
            sendSessionError(client_socket, 3);
            close(client_socket);
            return;
        }
        
        logger.info("Token válido para usuario: " + username);
        
        // 3. Crear mensaje sin token para storage
        ListSensorRequestWithoutToken storageMsg = createStorageListRequest();
        
        // 4. Enviar al storage y obtener respuesta
        std::vector<uint8_t> storageResponse = queryStorageForSensorList(storageMsg);
        
        // 5. Reenviar respuesta al cliente sin modificar
        if (!storageResponse.empty()) {
            send_message(client_socket, storageResponse.data(), storageResponse.size());
            logger.success("ListSensorResponse reenviada al cliente");
        } else {
            logger.error("Error obteniendo lista de sensores del storage");
        }
        
        close(client_socket);
    }
    
    // Handler para solicitud de datos de sensor
    void onDataRequest(const std::vector<uint8_t>& buf, int client_socket) {
        // 1. Deserializar mensaje del cliente
        auto clientMsg = DataRequest::deserialize(buf);
        
        std::string sensorId(clientMsg.sensor_id, strnlen(clientMsg.sensor_id, 16));
        logger.info("DataRequest recibido - Sensor: " + sensorId + 
                    " | StartDate: " + std::to_string(clientMsg.startDate) + 
                    " | EndDate: " + std::to_string(clientMsg.endDate));
        
        // 2. Validar token
        std::string username;
        if (!validateToken(clientMsg.token, username)) {
            logger.warning("Token inválido, rechazando solicitud de datos");
            sendSessionError(client_socket, 3);
            close(client_socket);
            return;
        }
        
        logger.info("Token válido para usuario: " + username);
        
        // 3. Crear mensaje sin token para storage
        DataRequestWithoutToken storageMsg = createStorageRequest(clientMsg);
        
        // 4. Enviar al storage y obtener respuesta
        std::vector<uint8_t> storageResponse = queryStorage(storageMsg);
        
        // 5. Reenviar respuesta al cliente
        if (!storageResponse.empty()) {
            send_message(client_socket, storageResponse.data(), storageResponse.size());
            logger.success("Respuesta de datos reenviada al cliente");
        } else {
            logger.error("Error obteniendo respuesta del storage");
        }
        
        close(client_socket);
    }
    
    // Crear mensaje sin token para storage
    DataRequestWithoutToken createStorageRequest(const DataRequest& clientMsg) {
        DataRequestWithoutToken storageMsg;
        storageMsg.message_id = MSG_DATA_REQUEST;
        
        // Copiar sensor_id
        std::memcpy(storageMsg.sensor_id, clientMsg.sensor_id, 16);
        
        // Copiar fechas
        storageMsg.startDate = clientMsg.startDate;
        storageMsg.endDate = clientMsg.endDate;
        
        return storageMsg;
    }
    
    // Consultar storage y obtener respuesta
    std::vector<uint8_t> queryStorage(const DataRequestWithoutToken& request) {
        StorageEndpoint* storage = selectActiveStorage();
        if (!storage) {
            logger.error("No hay storage disponible para la consulta");
            return {};
        }
        
        auto requestData = request.serialize();
        int storageSock = connect_to(storage->ip, storage->port);
        
        if (storageSock < 0) {
            logger.error("Error conectando con Storage " + storage->ip);
            return {};
        }
        
        if (!send_message(storageSock, requestData.data(), requestData.size())) {
            logger.error("Error enviando mensaje al Storage");
            close(storageSock);
            return {};
        }
        
        // Recibir respuesta del storage
        std::vector<uint8_t> response(8192);
        ssize_t bytes = recv_message(storageSock, response.data(), response.size());
        close(storageSock);
        
        if (bytes <= 0) {
            logger.error("Error recibiendo respuesta del Storage");
            return {};
        }
        
        response.resize(bytes);
        logger.info("Respuesta recibida del storage (" + std::to_string(bytes) + " bytes)");
        
        return response;
    }
    
    // Crear mensaje sin token para solicitar lista de sensores
    ListSensorRequestWithoutToken createStorageListRequest() {
        ListSensorRequestWithoutToken storageMsg;
        storageMsg.message_id = MSG_LIST_SENSOR_REQUEST;
        return storageMsg;
    }
    
    // Consultar storage para obtener lista de sensores
    std::vector<uint8_t> queryStorageForSensorList(const ListSensorRequestWithoutToken& request) {
        StorageEndpoint* storage = selectActiveStorage();
        if (!storage) {
            logger.error("No hay storage disponible para lista de sensores");
            return {};
        }
        
        auto requestData = request.serialize();
        int storageSock = connect_to(storage->ip, storage->port);
        
        if (storageSock < 0) {
            logger.error("Error conectando con Storage " + storage->ip);
            return {};
        }
        
        if (!send_message(storageSock, requestData.data(), requestData.size())) {
            logger.error("Error enviando mensaje al Storage para lista de sensores");
            close(storageSock);
            return {};
        }
        
        // Recibir respuesta del storage
        std::vector<uint8_t> response(4096);
        ssize_t bytes = recv_message(storageSock, response.data(), response.size());
        close(storageSock);
        
        if (bytes <= 0) {
            logger.error("Error recibiendo lista de sensores del Storage");
            return {};
        }
        
        response.resize(bytes);
        logger.success("Lista de sensores recibida del storage (" + std::to_string(bytes) + " bytes)");
        
        return response;
    }

    // Handler para solicitud de logs
    void onLogRequest(const std::vector<uint8_t>& buf, int client_socket) {
        auto clientMsg = LogRequest::deserialize(buf);
        
        logger.info("LogRequest recibido - Node Type: " + std::to_string(clientMsg.node_type));
        
        // Validar token
        std::string username;
        if (!validateToken(clientMsg.token, username)) {
            logger.warning("Token inválido, rechazando solicitud de logs");
            sendSessionError(client_socket, 3);
            close(client_socket);
            return;
        }
        
        logger.info("Token válido para usuario: " + username);
        
        // Si es para este nodo (PROXY), usar métodos de clase base
        if (clientMsg.node_type == NODE_PROXY) {
            std::vector<std::string> logs = getLogsInRange(clientMsg.startDate, clientMsg.endDate);
            sendLogResponse(client_socket, NODE_PROXY, logs);
            logger.success("Logs enviados al cliente: " + std::to_string(logs.size()) + " entradas");
        } else {
            // TODO: Forward to other nodes (Auth, Storage)
            logger.warning("Solicitud de logs para otro nodo no implementada aún");
            sendLogResponse(client_socket, clientMsg.node_type, {});
        }
        
        close(client_socket);
    }
};
