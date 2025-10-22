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

    MSG_STORAGE_SAVE = 9,           // Storage: guardar datos
    MSG_STORAGE_RESPONSE = 10,      // Storage: confirmación
    MSG_STORAGE_ERROR = 11,         // Storage: error
    MSG_STORAGE_SYNC_REQUEST = 12,  // Storage: solicitud sincronización
    MSG_STORAGE_SYNC_RESPONSE = 13, // Storage: respuesta sincronización
    MSG_STORAGE_SYNC_ERROR = 14,    // Storage: error sincronización
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

// --------------------------------------------------
// Mensaje de Storage
// --------------------------------------------------

// Estructura auxiliar para bloques de datos
struct SensorDataBlock {
    uint8_t sensorId;
    uint64_t date;
    uint64_t time;
    uint8_t dataLength;
    std::vector<uint8_t> data;
};

// StorageSave (ID 9)
struct StorageSave {
    uint8_t message_id = MSG_STORAGE_SAVE;
    uint8_t sensorId;
    uint64_t date;      // 8 bytes
    uint64_t time;      // 8 bytes
    uint8_t dataLength;
    std::vector<uint8_t> data;

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> result;
        result.push_back(message_id);
        result.push_back(sensorId);

        // Serializar date (8 bytes, big-endian)
        for (int i = 7; i >= 0; i--) {
            result.push_back((date >> (i * 8)) & 0xFF);
        }

        // Serializar time (8 bytes, big-endian)
        for (int i = 7; i >= 0; i--) {
            result.push_back((time >> (i * 8)) & 0xFF);
        }

        result.push_back(dataLength);
        result.insert(result.end(), data.begin(), data.end());

        return result;
    }

    static StorageSave deserialize(const std::vector<uint8_t>& buffer) {
        StorageSave msg;
        size_t idx = 0;

        msg.message_id = buffer[idx++];
        msg.sensorId = buffer[idx++];

        // Deserializar date
        msg.date = 0;
        for (int i = 0; i < 8; i++) {
            msg.date = (msg.date << 8) | buffer[idx++];
        }

        // Deserializar time
        msg.time = 0;
        for (int i = 0; i < 8; i++) {
            msg.time = (msg.time << 8) | buffer[idx++];
        }

        msg.dataLength = buffer[idx++];
        msg.data.assign(buffer.begin() + idx, buffer.begin() + idx + msg.dataLength);

        return msg;
    }
};

// StorageResponse (ID 10)
#pragma pack(push, 1)
struct StorageResponse {
    uint8_t message_id = MSG_STORAGE_RESPONSE;
    uint8_t statusCode;  // 0: Done, 1: Error

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data(sizeof(StorageResponse));
        std::memcpy(data.data(), this, sizeof(StorageResponse));
        return data;
    }

    static StorageResponse deserialize(const std::vector<uint8_t>& buffer) {
        StorageResponse msg;
        if (buffer.size() >= sizeof(StorageResponse)) {
            std::memcpy(&msg, buffer.data(), sizeof(StorageResponse));
        }
        return msg;
    }
};
#pragma pack(pop)

// StorageError (ID 11)
#pragma pack(push, 1)
struct StorageError {
    uint8_t message_id = MSG_STORAGE_ERROR;
    uint16_t errorCode;  // 401: Full, 402: Write failed, 403: Invalid data

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data;
        data.push_back(message_id);
        data.push_back((errorCode >> 8) & 0xFF);  // High byte
        data.push_back(errorCode & 0xFF);         // Low byte
        return data;
    }

    static StorageError deserialize(const std::vector<uint8_t>& buffer) {
        StorageError msg;
        if (buffer.size() >= 3) {
            msg.message_id = buffer[0];
            msg.errorCode = (buffer[1] << 8) | buffer[2];
        }
        return msg;
    }
};
#pragma pack(pop)

// StorageSyncRequest (ID 12)
#pragma pack(push, 1)
struct StorageSyncRequest {
    uint8_t message_id = MSG_STORAGE_SYNC_REQUEST;
    uint64_t startDate;
    uint64_t endDate;
    uint8_t sensorId;

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> result;
        result.push_back(message_id);

        // startDate
        for (int i = 7; i >= 0; i--) {
            result.push_back((startDate >> (i * 8)) & 0xFF);
        }

        // endDate
        for (int i = 7; i >= 0; i--) {
            result.push_back((endDate >> (i * 8)) & 0xFF);
        }

        result.push_back(sensorId);
        return result;
    }

    static StorageSyncRequest deserialize(const std::vector<uint8_t>& buffer) {
        StorageSyncRequest msg;
        size_t idx = 0;

        msg.message_id = buffer[idx++];

        msg.startDate = 0;
        for (int i = 0; i < 8; i++) {
            msg.startDate = (msg.startDate << 8) | buffer[idx++];
        }

        msg.endDate = 0;
        for (int i = 0; i < 8; i++) {
            msg.endDate = (msg.endDate << 8) | buffer[idx++];
        }

        msg.sensorId = buffer[idx++];
        return msg;
    }
};
#pragma pack(pop)

// StorageSyncResponse (ID 13)
struct StorageSyncResponse {
    uint8_t message_id = MSG_STORAGE_SYNC_RESPONSE;
    uint8_t dataLength;
    std::vector<SensorDataBlock> sensorDataBlocks;

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> result;
        result.push_back(message_id);
        result.push_back(dataLength);

        // Serializar cada bloque
        for (const auto& block : sensorDataBlocks) {
            result.push_back(block.sensorId);

            // date
            for (int i = 7; i >= 0; i--) {
                result.push_back((block.date >> (i * 8)) & 0xFF);
            }

            // time
            for (int i = 7; i >= 0; i--) {
                result.push_back((block.time >> (i * 8)) & 0xFF);
            }

            result.push_back(block.dataLength);
            result.insert(result.end(), block.data.begin(), block.data.end());
        }

        return result;
    }

    static StorageSyncResponse deserialize(const std::vector<uint8_t>& buffer) {
        StorageSyncResponse msg;
        size_t idx = 0;

        msg.message_id = buffer[idx++];
        msg.dataLength = buffer[idx++];

        for (int i = 0; i < msg.dataLength && idx < buffer.size(); i++) {
            SensorDataBlock block;

            block.sensorId = buffer[idx++];

            block.date = 0;
            for (int j = 0; j < 8; j++) {
                block.date = (block.date << 8) | buffer[idx++];
            }

            block.time = 0;
            for (int j = 0; j < 8; j++) {
                block.time = (block.time << 8) | buffer[idx++];
            }

            block.dataLength = buffer[idx++];
            block.data.assign(buffer.begin() + idx, buffer.begin() + idx + block.dataLength);
            idx += block.dataLength;

            msg.sensorDataBlocks.push_back(block);
        }

        return msg;
    }
};

// StorageSyncError (ID 14)
#pragma pack(push, 1)
struct StorageSyncError {
    uint8_t message_id = MSG_STORAGE_SYNC_ERROR;
    uint16_t errorCode;  // 404: Data not found

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data;
        data.push_back(message_id);
        data.push_back((errorCode >> 8) & 0xFF);
        data.push_back(errorCode & 0xFF);
        return data;
    }

    static StorageSyncError deserialize(const std::vector<uint8_t>& buffer) {
        StorageSyncError msg;
        if (buffer.size() >= 3) {
            msg.message_id = buffer[0];
            msg.errorCode = (buffer[1] << 8) | buffer[2];
        }
        return msg;
    }
};
#pragma pack(pop)
