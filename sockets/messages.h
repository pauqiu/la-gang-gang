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
    MSG_TOKEN_REGISTER = 4,  // Auth -> Proxy: registrar token válido
    MSG_SESSION_VALIDATE = 5, // Client -> Proxy: validar sesión con token
    MSG_SESSION_OK = 6,       // Proxy -> Client: sesión válida
    MSG_SESSION_ERROR = 7,    // Proxy -> Client: sesión inválida
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

    // Serializa a bytes
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

// AuthError - Respuesta de error de autenticación (ID 3)
// Tamaño: 2 bytes (1 byte id + 1 byte error_code)
// Error codes:
//   - 1: Credenciales incorrectas
//   - 2: Exceso de intentos de autenticación
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

// TokenNotif - Auth envía token al Proxy (ID 4)
// Tamaño: 50 bytes (1 byte id + 32 bytes token + 16 bytes username + 1 byte role)
#pragma pack(push, 1)
struct TokenNotif {
    uint8_t message_id = MSG_TOKEN_REGISTER;
    uint8_t token[32];
    char username[16];
    uint8_t role;

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data(sizeof(TokenNotif));
        std::memcpy(data.data(), this, sizeof(TokenNotif));
        return data;
    }

    static TokenNotif deserialize(const std::vector<uint8_t>& buffer) {
        TokenNotif msg;
        if (buffer.size() >= sizeof(TokenNotif)) {
            std::memcpy(&msg, buffer.data(), sizeof(TokenNotif));
        }
        return msg;
    }
};
#pragma pack(pop)

// SessionValidate - Cliente valida sesión con Proxy (ID 5)
// Tamaño: 49 bytes (1 byte id + 16 bytes username + 32 bytes token)
#pragma pack(push, 1)
struct SessionValidate {
    uint8_t message_id = MSG_SESSION_VALIDATE;
    char username[16];
    uint8_t token[32];

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data(sizeof(SessionValidate));
        std::memcpy(data.data(), this, sizeof(SessionValidate));
        return data;
    }

    static SessionValidate deserialize(const std::vector<uint8_t>& buffer) {
        SessionValidate msg;
        if (buffer.size() >= sizeof(SessionValidate)) {
            std::memcpy(&msg, buffer.data(), sizeof(SessionValidate));
        }
        return msg;
    }
};
#pragma pack(pop)

// SessionOk - Proxy confirma sesión válida (ID 6)
// Tamaño: 2 bytes (1 byte id + 1 byte role)
#pragma pack(push, 1)
struct SessionOk {
    uint8_t message_id = MSG_SESSION_OK;
    uint8_t role;

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data(sizeof(SessionOk));
        std::memcpy(data.data(), this, sizeof(SessionOk));
        return data;
    }

    static SessionOk deserialize(const std::vector<uint8_t>& buffer) {
        SessionOk msg;
        if (buffer.size() >= sizeof(SessionOk)) {
            std::memcpy(&msg, buffer.data(), sizeof(SessionOk));
        }
        return msg;
    }
};
#pragma pack(pop)

// SessionError - Proxy rechaza sesión (ID 7)
// Tamaño: 2 bytes (1 byte id + 1 byte error_code)
// Error codes:
//   - 3: Token inválido o expirado
#pragma pack(push, 1)
struct SessionError {
    uint8_t message_id = MSG_SESSION_ERROR;
    uint8_t error_code;

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data(sizeof(SessionError));
        std::memcpy(data.data(), this, sizeof(SessionError));
        return data;
    }

    static SessionError deserialize(const std::vector<uint8_t>& buffer) {
        SessionError msg;
        if (buffer.size() >= sizeof(SessionError)) {
            std::memcpy(&msg, buffer.data(), sizeof(SessionError));
        }
        return msg;
    }
};
#pragma pack(pop)
