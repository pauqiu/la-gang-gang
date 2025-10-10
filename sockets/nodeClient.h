#pragma once
#include "communication.h"
#include "messages.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include <cstring>

class NodeClient {
public:
    void sendAuthentication(const std::string& user, const std::string& pass, int tries = 0) {
        currentUsername = user; // Guardar username para validación posterior
        
        int sock = connectToAuthServer();
        if (sock < 0) return;

        if (!sendAuthRequest(sock, user, pass, tries)) {
            close(sock);
            return;
        }

        auto response = receiveAuthResponse(sock);
        if (!response.empty()) {
            processAuthResponse(response);
        }

        close(sock);
    }
    
    bool validateSessionWithProxy() {
        if (!hasValidToken()) {
            std::cerr << "[Client] No hay token válido.\n";
            return false;
        }
        
        int sock = connectToProxy();
        if (sock < 0) return false;
        
        if (!sendSessionValidate(sock)) {
            close(sock);
            return false;
        }
        
        auto response = receiveResponse(sock);
        bool isValid = false;
        if (!response.empty()) {
            isValid = processSessionResponse(response);
        }
        
        close(sock);
        return isValid;
    }
    
    bool hasValidToken() const {
        return tokenReceived;
    }
    
    uint8_t getRole() const {
        return currentRole;
    }

private:
    uint8_t sessionToken[32] = {0};
    uint8_t currentRole = 0;
    bool tokenReceived = false;
    std::string currentUsername;

    int connectToAuthServer() {
        int sock = connect_to("127.0.0.1", 5001);
        if (sock < 0) {
            std::cerr << "[Client] Error al conectar con Auth.\n";
        }
        return sock;
    }
    
    int connectToProxy() {
        int sock = connect_to("127.0.0.1", 5002);
        if (sock < 0) {
            std::cerr << "[Client] Error al conectar con Proxy.\n";
        }
        return sock;
    }

    bool sendAuthRequest(int sock, const std::string& user, const std::string& pass, int tries) {
        AuthMessage msg;
        msg.user = user;
        msg.password = pass;
        msg.failedTries = tries;

        auto data = msg.serialize();
        if (!send_message(sock, data.data(), data.size())) {
            std::cerr << "[Client] Error al enviar mensaje.\n";
            return false;
        }
        
        std::cout << "[Client] Mensaje de autenticación enviado. Esperando respuesta...\n";
        return true;
    }

    std::vector<uint8_t> receiveAuthResponse(int sock) {
        std::vector<uint8_t> response(1024);
        ssize_t bytes = recv_message(sock, response.data(), response.size());
        
        if (bytes <= 0) {
            std::cerr << "[Client] Error al recibir respuesta del servidor.\n";
            return {};
        }
        
        response.resize(bytes);
        std::cout << "[Client] Recibidos " << bytes << " bytes, ID: " << (int)response[0] << "\n";
        return response;
    }

    void processAuthResponse(const std::vector<uint8_t>& response) {
        if (response[0] == MSG_AUTH_RESPONSE) {
            handleAuthResponse(response);
        } else if (response[0] == MSG_AUTH_ERROR) {
            handleAuthError(response);
        } else {
            std::cerr << "[Client] Respuesta desconocida con ID: " << (int)response[0] << "\n";
        }
    }

    void handleAuthResponse(const std::vector<uint8_t>& data) {
        auto response = AuthResponse::deserialize(data);
        
        // Guardar token y rol
        std::memcpy(sessionToken, response.token, 32);
        currentRole = response.role;
        tokenReceived = true;
        
        std::cout << "\nAUTENTICACIÓN EXITOSA\n";
        std::cout << "Rol asignado: " << (int)response.role << "\n";
        std::cout << "Token de sesión (hex): ";
        printToken(response.token);
        std::cout << "\n";
    }
    
    void handleAuthError(const std::vector<uint8_t>& data) {
        auto error = AuthError::deserialize(data);
        
        std::cout << "\nERROR DE AUTENTICACIÓN\n";
        std::cout << "Código de error: " << (int)error.error_code << " - ";
        std::cout << getErrorMessage(error.error_code) << "\n\n";
    }

    void printToken(const uint8_t token[32]) {
        for (int i = 0; i < 32; i++) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)token[i];
        }
        std::cout << std::dec;
    }

    std::string getErrorMessage(uint8_t errorCode) {
        switch (errorCode) {
            case 1: return "Credenciales incorrectas";
            case 2: return "Exceso de intentos de autenticación";
            case 3: return "Token inválido o expirado";
            default: return "Error desconocido";
        }
    }
    
    bool sendSessionValidate(int sock) {
        SessionValidate msg;
        msg.message_id = MSG_SESSION_VALIDATE;
        
        // Copiar username (máximo 16 bytes)
        std::memset(msg.username, 0, 16);
        size_t len = std::min(currentUsername.length(), size_t(16));
        std::memcpy(msg.username, currentUsername.c_str(), len);
        
        // Copiar token
        std::memcpy(msg.token, sessionToken, 32);
        
        auto data = msg.serialize();
        if (!send_message(sock, data.data(), data.size())) {
            std::cerr << "[Client] Error al enviar validación de sesión.\n";
            return false;
        }
        
        std::cout << "[Client] Validación de sesión enviada al Proxy (user=" << currentUsername << ")...\n";
        return true;
    }
    
    std::vector<uint8_t> receiveResponse(int sock) {
        std::vector<uint8_t> response(1024);
        ssize_t bytes = recv_message(sock, response.data(), response.size());
        
        if (bytes <= 0) {
            std::cerr << "[Client] Error al recibir respuesta.\n";
            return {};
        }
        
        response.resize(bytes);
        return response;
    }
    
    bool processSessionResponse(const std::vector<uint8_t>& response) {
        if (response[0] == MSG_SESSION_OK) {
            auto msg = SessionOk::deserialize(response);
            std::cout << "\nSESIÓN VÁLIDA\n";
            std::cout << "Acceso concedido con rol: " << (int)msg.role << "\n";
            return true;
        } else if (response[0] == MSG_SESSION_ERROR) {
            auto msg = SessionError::deserialize(response);
            std::cout << "\nSESIÓN INVÁLIDA\n";
            std::cout << "Error: " << getErrorMessage(msg.error_code) << "\n\n";
            return false;
        } else {
            std::cerr << "[Client] Respuesta desconocida del Proxy.\n";
            return false;
        }
    }
};
