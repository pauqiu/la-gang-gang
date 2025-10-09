#pragma once
#include "communication.h"
#include "messages.h"
#include <iostream>
#include <vector>
#include <iomanip>

class NodeClient {
public:
    void sendAuthentication(const std::string& user, const std::string& pass, int tries = 0) {
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

private:
    int connectToAuthServer() {
        int sock = connect_to("127.0.0.1", 5001);
        if (sock < 0) {
            std::cerr << "[Client] Error al conectar con Auth.\n";
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
            default: return "Error desconocido";
        }
    }
};
