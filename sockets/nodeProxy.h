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
};
