#include "node_base.h"
#include "messages.h"
#include "security.h"
#include "filesystem.h"
#include "../include/logger.h"
#include "endpoints.h"
#include <iostream>
#include <cstring>
#include <random>

class NodeAuth : public NodeBase {
public:
    NodeAuth(int port, Security* securityInstance, FileSystem* fs)
        : NodeBase(port), security(securityInstance), logger(fs, "aLogs.bin") {
        // Configurar soporte de logs usando método de clase base
        setupLogSupport(fs, "aLogs.bin", NODE_AUTH);
        
        dispatcher.registerHandler(MSG_AUTHENTICATION,
                                   [this](const std::vector<uint8_t>& buf, int client_socket) {
                                       onAuthentication(buf, client_socket);
                                   });
        
        dispatcher.registerHandler(MSG_LOG_REQUEST,
                                   [this](const std::vector<uint8_t>& buf, int client_socket) {
                                       onLogRequest(buf, client_socket);
                                   });
    }

    void onAuthentication(const std::vector<uint8_t>& buf, int client_socket) {
        auto msg = AuthMessage::deserialize(buf);
        std::cout << "[AuthNode] Usuario: " << msg.user << ", Password: " << msg.password << "\n";

        // Validar credenciales usando Security
        bool credentialsValid = false;
        uint8_t userRole = 0;
        uint8_t errorCode = 0;
        validateCredentials(msg, credentialsValid, userRole, errorCode);

        // Enviar respuesta
        if (credentialsValid) {
            sendSuccessResponse(client_socket, msg.user, userRole);
            logger.success("Autenticación exitosa - Usuario: " + msg.user +
                           " | Rol: " + std::to_string((int)userRole));
        } else {
            sendErrorResponse(client_socket, errorCode);
            logger.warning("Intento de autenticación fallido - Usuario: " + msg.user +
                           " | Error code: " + std::to_string((int)errorCode));
        }

        close(client_socket);
    }

private:
    Security* security;  // Puntero a Security
    Logger logger;


    void validateCredentials(const AuthMessage& msg, bool& valid,
                             uint8_t& role, uint8_t& errorCode) {
        QString username = QString::fromStdString(msg.user);
        QString password = QString::fromStdString(msg.password);

        // Usar Security::verifyUser - ya retorna el número de rol
        int result = security->verifyUser(username, password);

        if (result >= 0) {
            valid = true;

            role = static_cast<uint8_t>(result);  // verifyUser ya retorna 1-7

            QString roleStr = security->getUserRole(username);  // Solo para logging

            std::string logMsg = "Credenciales validadas - Usuario: " + msg.user +
                                 " | Rol: " + roleStr.toStdString() +
                                 " (" + std::to_string((int)role) + ")";
            logger.info(logMsg);
        } else {
            valid = false;
            errorCode = 1;
            logger.warning("Validación fallida - Usuario: " + msg.user);
        }
    }

    void sendSuccessResponse(int client_socket, const std::string& username, uint8_t role) {
        AuthResponse response;
        response.message_id = MSG_AUTH_RESPONSE;
        response.role = role;
        generateSessionToken(response.token);

        // Enviar respuesta al cliente
        auto data = response.serialize();
        send_message(client_socket, data.data(), data.size());
        std::cout << "[AuthNode] AuthResponse enviado al cliente (role=" << (int)role << ")\n";

        // Registrar token en el proxy
        registerTokenWithProxy(username, response.token, role);
    }

    void registerTokenWithProxy(const std::string& username, const uint8_t token[32], uint8_t role) {
        TokenNotif tokenMsg;
        tokenMsg.message_id = MSG_TOKEN_REGISTER;
        std::memcpy(tokenMsg.token, token, 32);

        // Copiar username (máximo 16 bytes)
        std::memset(tokenMsg.username, 0, 16);
        size_t len = std::min(username.length(), size_t(16));
        std::memcpy(tokenMsg.username, username.c_str(), len);

        tokenMsg.role = role;

        // Enviar token al proxy (puerto 5002)
        auto data = tokenMsg.serialize();
        sendTo(getProxyIp(), getProxyPort(), data);
        std::cout << "[AuthNode] TokenNotif enviado a Proxy (user=" << username << ", role=" << (int)role << ")\n";
    }

    void sendErrorResponse(int client_socket, uint8_t errorCode) {
        AuthError error;
        error.message_id = MSG_AUTH_ERROR;
        error.error_code = errorCode;

        auto data = error.serialize();
        send_message(client_socket, data.data(), data.size());
        std::cout << "[AuthNode] AuthError enviado (error_code=" << (int)errorCode << ")\n";
    }

    void generateSessionToken(uint8_t token[32]) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 255);
        for (int i = 0; i < 32; i++) {
            token[i] = static_cast<uint8_t>(dis(gen));
        }
    }
    
    // Handler para solicitud de logs
    void onLogRequest(const std::vector<uint8_t>& buf, int client_socket) {
        auto clientMsg = LogRequest::deserialize(buf);
        
        logger.info("LogRequest recibido - Node Type: " + std::to_string(clientMsg.node_type));
        
        // Auth no valida tokens (es el que los genera), procesa directamente
        if (clientMsg.node_type == NODE_AUTH) {
            std::vector<std::string> logs = getLogsInRange(clientMsg.startDate, clientMsg.endDate);
            sendLogResponse(client_socket, NODE_AUTH, logs);
            logger.success("Logs enviados al cliente: " + std::to_string(logs.size()) + " entradas");
        } else {
            logger.warning("Solicitud de logs para otro nodo recibida en Auth");
            sendLogResponse(client_socket, clientMsg.node_type, {});
        }
        
        close(client_socket);
    }
};
