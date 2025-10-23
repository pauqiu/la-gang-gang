#pragma once
#include "node_base.h"
#include "messages.h"
#include <iostream>
#include <map>
#include <cstring>
#include <array>

class NodeProxy : public NodeBase {
public:
    NodeProxy(int port) : NodeBase(port) {
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
    }

private:
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
        std::cout << "[Proxy] TokenNotif recibido - User: " << username 
                  << ", Role: " << (int)msg.role 
                  << ", Token: " << tokenHex.substr(0, 16) << "...\n";
        
        close(client_socket);
    }

    // Validar sesión recibida de client
    void onSessionValidate(const std::vector<uint8_t>& buf, int client_socket) {
        auto msg = SessionValidate::deserialize(buf);
        
        // Extraer username
        std::string username(msg.username, strnlen(msg.username, 16));
        std::string tokenHex = tokenToString(msg.token);
        
        std::cout << "[Proxy] Validando sesión - User: " << username 
                  << ", Token: " << tokenHex.substr(0, 16) << "...\n";
        
        // TODO: Buscar token en filesystem en lugar de memoria
        if (isTokenValid(username, msg.token)) {
            sendSessionOk(client_socket, validTokens[username].role);
        } else {
            sendSessionError(client_socket, 3);
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
        std::cout << "[Proxy] SessionOk enviado (role=" << (int)role << ")\n";
    }

    void sendSessionError(int client_socket, uint8_t errorCode) {
        SessionError error;
        error.message_id = MSG_SESSION_ERROR;
        error.error_code = errorCode;
        
        auto data = error.serialize();
        send_message(client_socket, data.data(), data.size());
        std::cout << "[Proxy] SessionError enviado (error_code=" << (int)errorCode << ")\n";
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
        auto msg = ListSensorRequest::deserialize(buf);
        
        std::cout << "[Proxy] ListSensorRequest recibido\n";
        
        // Validar token
        std::string username;
        if (!validateToken(msg.token, username)) {
            std::cout << "[Proxy] Token inválido, rechazando solicitud\n";
            sendSessionError(client_socket, 3);
            close(client_socket);
            return;
        }
        
        std::cout << "[Proxy] Token válido para usuario: " << username << "\n";
        
        // TODO: Consultar storage para obtener lista real de sensores
        // Por ahora, respuesta vacía o hardcoded
        ListSensorResponse response;
        response.message_id = MSG_LIST_SENSOR_RESPONSE;
        response.sensorCount = 0;
        
        auto data = response.serialize();
        send_message(client_socket, data.data(), data.size());
        std::cout << "[Proxy] ListSensorResponse enviado\n";
        
        close(client_socket);
    }
    
    // Handler para solicitud de datos de sensor
    void onDataRequest(const std::vector<uint8_t>& buf, int client_socket) {
        // 1. Deserializar mensaje del cliente
        auto clientMsg = DataRequest::deserialize(buf);
        
        std::string sensorId(clientMsg.sensor_id, strnlen(clientMsg.sensor_id, 16));
        std::cout << "[Proxy] DataRequest recibido - Sensor: " << sensorId 
                  << ", StartDate: " << clientMsg.startDate 
                  << ", EndDate: " << clientMsg.endDate << "\n";
        
        // 2. Validar token
        std::string username;
        if (!validateToken(clientMsg.token, username)) {
            std::cout << "[Proxy] Token inválido, rechazando solicitud\n";
            sendSessionError(client_socket, 3);
            close(client_socket);
            return;
        }
        
        std::cout << "[Proxy] Token válido para usuario: " << username << "\n";
        
        // 3. Crear mensaje sin token para storage
        DataRequestWithoutToken storageMsg = createStorageRequest(clientMsg);
        
        // 4. Enviar al storage y obtener respuesta
        std::vector<uint8_t> storageResponse = queryStorage(storageMsg);
        
        // 5. Reenviar respuesta al cliente
        if (!storageResponse.empty()) {
            send_message(client_socket, storageResponse.data(), storageResponse.size());
            std::cout << "[Proxy] Respuesta reenviada al cliente\n";
        } else {
            std::cerr << "[Proxy] Error obteniendo respuesta del storage\n";
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
        std::cout << "[Proxy] Enviando DataRequestWithoutToken al storage (sin token)\n";
        
        // Serializar y enviar al storage
        auto requestData = request.serialize();
        int storageSock = connect_to("127.0.0.1", 5003);
        
        if (storageSock < 0) {
            std::cerr << "[Proxy] Error conectando con Storage\n";
            return {};
        }
        
        if (!send_message(storageSock, requestData.data(), requestData.size())) {
            std::cerr << "[Proxy] Error enviando mensaje al Storage\n";
            close(storageSock);
            return {};
        }
        
        // Recibir respuesta del storage
        std::vector<uint8_t> response(8192);
        ssize_t bytes = recv_message(storageSock, response.data(), response.size());
        close(storageSock);
        
        if (bytes <= 0) {
            std::cerr << "[Proxy] Error recibiendo respuesta del Storage\n";
            return {};
        }
        
        response.resize(bytes);
        std::cout << "[Proxy] Respuesta recibida del storage (" << bytes << " bytes)\n";
        
        return response;
    }
};
