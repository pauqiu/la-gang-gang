#pragma once
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>

// Tipo de mensaje
enum MessageType : uint8_t {
    MSG_AUTHENTICATION = 1,
    MSG_AUTH_RESPONSE  = 2,
    MSG_AUTH_ERROR     = 3,
};

struct Message {
    uint8_t id;
    std::vector<uint8_t> payload;
};

// --------------------------------------------------
// Mensaje de autenticación
// --------------------------------------------------
struct AuthMessage {
    uint8_t id = MSG_AUTHENTICATION;
    std::string user;
    std::string password;
    uint8_t failedTries = 0;

    // Serializa la estructura a bytes
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data;
        data.push_back(id);
        data.push_back(user.size());
        data.insert(data.end(), user.begin(), user.end());
        data.push_back(password.size());
        data.insert(data.end(), password.begin(), password.end());
        data.push_back(failedTries);
        return data;
    }

    // Deserializa desde bytes
    static AuthMessage deserialize(const std::vector<uint8_t>& buffer) {
        AuthMessage msg;
        size_t i = 1; // [0] es messageId
        uint8_t lenUser = buffer[i++];
        msg.user.assign((char*)&buffer[i], lenUser);
        i += lenUser;
        uint8_t lenPass = buffer[i++];
        msg.password.assign((char*)&buffer[i], lenPass);
        i += lenPass;
        msg.failedTries = buffer[i];
        return msg;
    }
};

// AuthResponse - Respuesta exitosa de autenticación (ID 2)
// Tamaño: 34 bytes (1 byte id + 32 bytes token + 1 byte role)
#pragma pack(push, 1)
struct AuthResponse {
    uint8_t message_id = MSG_AUTH_RESPONSE;  // 1 byte - ID fijo: 2
    uint8_t token[32];                        // 32 bytes - Token de sesión
    uint8_t role;                             // 1 byte - Rol del usuario (1-7)

    // Serializa la estructura a bytes
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data(sizeof(AuthResponse));
        std::memcpy(data.data(), this, sizeof(AuthResponse));
        return data;
    }

    // Deserializa desde bytes
    static AuthResponse deserialize(const std::vector<uint8_t>& buffer) {
        AuthResponse msg;
        if (buffer.size() >= sizeof(AuthResponse)) {
            std::memcpy(&msg, buffer.data(), sizeof(AuthResponse));
        }
        return msg;
    }
};
#pragma pack(pop)

// --------------------------------------------------
// AuthError - Respuesta de error de autenticación (ID 3)
// Tamaño: 2 bytes (1 byte id + 1 byte error_code)
// Error codes:
//   - 301: Credenciales incorrectas
//   - 302: Exceso de intentos de autenticación
// --------------------------------------------------
#pragma pack(push, 1)
struct AuthError {
    uint8_t message_id = MSG_AUTH_ERROR;     // 1 byte - ID fijo: 3
    uint8_t error_code;                       // 1 byte - Código de error

    // Serializa la estructura a bytes
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data(sizeof(AuthError));
        std::memcpy(data.data(), this, sizeof(AuthError));
        return data;
    }

    // Deserializa desde bytes
    static AuthError deserialize(const std::vector<uint8_t>& buffer) {
        AuthError msg;
        if (buffer.size() >= sizeof(AuthError)) {
            std::memcpy(&msg, buffer.data(), sizeof(AuthError));
        }
        return msg;
    }
};
#pragma pack(pop)
