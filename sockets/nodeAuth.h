#include "node_base.h"
#include "messages.h"
#include <iostream>
#include <cstring>
#include <random>

class NodeAuth : public NodeBase {
public:
    NodeAuth(int port) : NodeBase(port) {
        dispatcher.registerHandler(MSG_AUTHENTICATION, 
            [this](const std::vector<uint8_t>& buf, int client_socket) { 
                onAuthentication(buf, client_socket); 
            });
    }

    void onAuthentication(const std::vector<uint8_t>& buf, int client_socket) {
        auto msg = AuthMessage::deserialize(buf);
        std::cout << "[AuthNode] Usuario: " << msg.user << ", Password: " << msg.password << "\n";

        // Validar credenciales
        bool credentialsValid = false;
        uint8_t userRole = 0;
        uint8_t errorCode = 0;
        validateCredentials(msg, credentialsValid, userRole, errorCode);
        
        // Enviar respuesta
        if (credentialsValid) {
            sendSuccessResponse(client_socket, msg.user, userRole);
        } else {
            sendErrorResponse(client_socket, errorCode);
        }
        
        close(client_socket);
    }

private:
    // TODO: Integrar validación real con nuestro filesystem
    // Debe buscar usuario, comparar hash de contraseña, verificar intentos fallidos y obtener rol real
    void validateCredentials(const AuthMessage& msg, bool& valid, uint8_t& role, uint8_t& errorCode) {
        // Lógica temporal de prueba
        if (msg.user == "admin") {
            valid = true;
            role = 6;
            std::cout << "[AuthNode] Credenciales válidas - Usuario: admin, Rol: " << (int)role << "\n";
        } else if (msg.user == "fail") {
            valid = false;
            errorCode = 1;
            std::cout << "[AuthNode] Credenciales incorrectas\n";
        } else if (msg.user == "blocked") {
            valid = false;
            errorCode = 2;
            std::cout << "[AuthNode] Usuario bloqueado por exceso de intentos\n";
        } else {
            valid = false;
            errorCode = 1;
            std::cout << "[AuthNode] Usuario desconocido\n";
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
        sendTo("127.0.0.1", 5002, data);
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

    // TODO: generar token seguro
    void generateSessionToken(uint8_t token[32]) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 255);
        for (int i = 0; i < 32; i++) {
            token[i] = static_cast<uint8_t>(dis(gen));
        }
    }
};
