#pragma once
#include <string>
#include <vector>
#include <cstring>
#include <array>
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
    MSG_DATA_REQUEST = 8,     // Client -> Proxy: solicitud de datos de sensores

    MSG_STORAGE_SAVE = 9,           // Storage: guardar datos
    MSG_STORAGE_RESPONSE = 10,      // Storage: confirmación
    MSG_STORAGE_ERROR = 11,         // Storage: error
    MSG_STORAGE_SYNC_REQUEST = 12,  // Storage: solicitud sincronización
    MSG_STORAGE_SYNC_RESPONSE = 13, // Storage: respuesta sincronización
    MSG_STORAGE_SYNC_ERROR = 14,    // Storage: error sincronización
    MSG_DATA_RESPONSE = 15,          // Proxy -> Client: respuesta con datos de sensores
    MSG_LIST_SENSOR_REQUEST = 16,    // Client -> Proxy: solicitud de lista de sensores
    MSG_LIST_SENSOR_RESPONSE = 17,   // Proxy -> Client: respuesta con lista de sensores

    MSG_ULTRASONIC_SENSOR_DATA = 18,            // Arduino -> Receptor: datos detectados por los sensores
    MSG_TILT_SENSOR_DATA = 19,            // Arduino -> Receptor: datos detectados por los sensores
    MSG_SOUND_SENSOR_DATA = 20,            // Arduino -> Receptor: datos detectados por los sensores
    MSG_HUMIDITY_SENSOR_DATA = 21,            // Arduino -> Receptor: datos detectados por los sensores
    MSG_SENSORS_DATA = 22,
};

// Id de los sensores
enum sensor : uint8_t {
    ULTRASONIC_SENSOR = 1,
    TILT_SENSOR = 2,
    SOUND_SENSOR = 3,
    HUMIDITY_SENSOR = 4,
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

// Mensajes de consulta de datos (Client - Proxy)

// DataRequest - Cliente solicita datos de un sensor en un rango de fechas (ID 8)
// Tamaño: 65 bytes (1 byte id + 32 bytes token + 16 bytes sensor_id + 8 bytes startDate + 8 bytes endDate)
#pragma pack(push, 1)
struct DataRequest {
    uint8_t message_id = MSG_DATA_REQUEST;
    uint8_t token[32];     // Token de sesión para validación
    char sensor_id[16];    // ID del sensor solicitado (ej: "DHT11A", "PIR001")
    uint64_t startDate;    // Fecha inicio en formato YYYYMMDD (ej: 20250925)
    uint64_t endDate;      // Fecha fin en formato YYYYMMDD (ej: 20250925)

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> result;
        result.push_back(message_id);
        
        // Token (32 bytes)
        result.insert(result.end(), token, token + 32);
        
        // Sensor ID (16 bytes)
        result.insert(result.end(), sensor_id, sensor_id + 16);
        
        // StartDate (8 bytes, big-endian)
        for (int i = 7; i >= 0; i--) {
            result.push_back((startDate >> (i * 8)) & 0xFF);
        }
        
        // EndDate (8 bytes, big-endian)
        for (int i = 7; i >= 0; i--) {
            result.push_back((endDate >> (i * 8)) & 0xFF);
        }
        
        return result;
    }

    static DataRequest deserialize(const std::vector<uint8_t>& buffer) {
        DataRequest msg;
        size_t idx = 0;
        
        msg.message_id = buffer[idx++];
        
        // Token (32 bytes)
        std::memcpy(msg.token, &buffer[idx], 32);
        idx += 32;
        
        // Sensor ID (16 bytes)
        std::memcpy(msg.sensor_id, &buffer[idx], 16);
        idx += 16;
        
        // StartDate (8 bytes)
        msg.startDate = 0;
        for (int i = 0; i < 8; i++) {
            msg.startDate = (msg.startDate << 8) | buffer[idx++];
        }
        
        // EndDate (8 bytes)
        msg.endDate = 0;
        for (int i = 0; i < 8; i++) {
            msg.endDate = (msg.endDate << 8) | buffer[idx++];
        }
        
        return msg;
    }
};
#pragma pack(pop)

// DataRequestWithoutToken - Proxy envía al Storage (sin token)
// Tamaño: 33 bytes (1 byte id + 16 bytes sensor_id + 8 bytes startDate + 8 bytes endDate)
#pragma pack(push, 1)
struct DataRequestWithoutToken {
    uint8_t message_id = MSG_DATA_REQUEST;
    char sensor_id[16];    // ID del sensor solicitado (ej: "DHT11A", "PIR001")
    uint64_t startDate;    // Fecha inicio en formato YYYYMMDD (ej: 20250925)
    uint64_t endDate;      // Fecha fin en formato YYYYMMDD (ej: 20250925)

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> result;
        result.push_back(message_id);
        
        // Sensor ID (16 bytes)
        result.insert(result.end(), sensor_id, sensor_id + 16);
        
        // StartDate (8 bytes, big-endian)
        for (int i = 7; i >= 0; i--) {
            result.push_back((startDate >> (i * 8)) & 0xFF);
        }
        
        // EndDate (8 bytes, big-endian)
        for (int i = 7; i >= 0; i--) {
            result.push_back((endDate >> (i * 8)) & 0xFF);
        }
        
        return result;
    }

    static DataRequestWithoutToken deserialize(const std::vector<uint8_t>& buffer) {
        DataRequestWithoutToken msg;
        size_t idx = 0;
        
        msg.message_id = buffer[idx++];
        
        // Sensor ID (16 bytes)
        std::memcpy(msg.sensor_id, &buffer[idx], 16);
        idx += 16;
        
        // StartDate (8 bytes)
        msg.startDate = 0;
        for (int i = 0; i < 8; i++) {
            msg.startDate = (msg.startDate << 8) | buffer[idx++];
        }
        
        // EndDate (8 bytes)
        msg.endDate = 0;
        for (int i = 0; i < 8; i++) {
            msg.endDate = (msg.endDate << 8) | buffer[idx++];
        }
        
        return msg;
    }
};
#pragma pack(pop)

// Estructura auxiliar para una entrada de sensor en DataResponse
#pragma pack(push, 1)
struct SensorEntry {
    char sensor_id[16];    // ID del sensor (ej: "DHT11A", "PIR001")
    uint64_t date;         // Fecha (YYYYMMDD)
    uint64_t time;         // Hora (HHMMSS)
    float data_value;      // Valor del dato (temperatura, distancia, binario)
    char status[8];        // Estado: "NORMAL" o "ALERT"
};
#pragma pack(pop)

// DataResponse - Proxy responde con datos de sensores (ID 15)
// Tamaño variable: 3 bytes header + (44 bytes * entriesCount)
struct DataResponse {
    uint8_t message_id = MSG_DATA_RESPONSE;
    uint8_t entriesCount;  // Número de entradas de sensores
    std::vector<SensorEntry> entries;

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> result;
        result.push_back(message_id);
        result.push_back(entriesCount);
        
        // Serializar cada entrada
        for (const auto& entry : entries) {
            // sensor_id (16 bytes)
            result.insert(result.end(), entry.sensor_id, entry.sensor_id + 16);
            
            // date (8 bytes)
            for (int i = 7; i >= 0; i--) {
                result.push_back((entry.date >> (i * 8)) & 0xFF);
            }
            
            // time (8 bytes)
            for (int i = 7; i >= 0; i--) {
                result.push_back((entry.time >> (i * 8)) & 0xFF);
            }
            
            // data_value (4 bytes)
            uint32_t floatBits;
            std::memcpy(&floatBits, &entry.data_value, sizeof(float));
            for (int i = 3; i >= 0; i--) {
                result.push_back((floatBits >> (i * 8)) & 0xFF);
            }
            
            // status (8 bytes)
            result.insert(result.end(), entry.status, entry.status + 8);
        }
        
        return result;
    }

    static DataResponse deserialize(const std::vector<uint8_t>& buffer) {
        DataResponse msg;
        size_t idx = 0;
        
        msg.message_id = buffer[idx++];
        msg.entriesCount = buffer[idx++];
        
        for (int i = 0; i < msg.entriesCount && idx < buffer.size(); i++) {
            SensorEntry entry;
            
            // sensor_id (16 bytes)
            std::memcpy(entry.sensor_id, &buffer[idx], 16);
            idx += 16;
            
            // date (8 bytes)
            entry.date = 0;
            for (int j = 0; j < 8; j++) {
                entry.date = (entry.date << 8) | buffer[idx++];
            }
            
            // time (8 bytes)
            entry.time = 0;
            for (int j = 0; j < 8; j++) {
                entry.time = (entry.time << 8) | buffer[idx++];
            }
            
            // data_value (4 bytes)
            uint32_t floatBits = 0;
            for (int j = 0; j < 4; j++) {
                floatBits = (floatBits << 8) | buffer[idx++];
            }
            std::memcpy(&entry.data_value, &floatBits, sizeof(float));
            
            // status (8 bytes)
            std::memcpy(entry.status, &buffer[idx], 8);
            idx += 8;
            
            msg.entries.push_back(entry);
        }
        
        return msg;
    }
};

// Mensajes de lista de sensores (Client <-> Proxy)

// ListSensorRequest - Cliente solicita lista de sensores disponibles (ID 16)
// Tamaño: 33 bytes (1 byte id + 32 bytes token)
#pragma pack(push, 1)
struct ListSensorRequest {
    uint8_t message_id = MSG_LIST_SENSOR_REQUEST;
    uint8_t token[32];     // Token de sesión para validación

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> result;
        result.push_back(message_id);
        
        // Token (32 bytes)
        result.insert(result.end(), token, token + 32);
        
        return result;
    }

    static ListSensorRequest deserialize(const std::vector<uint8_t>& buffer) {
        ListSensorRequest msg;
        size_t idx = 0;
        
        msg.message_id = buffer[idx++];
        
        // Token (32 bytes)
        if (buffer.size() >= 33) {
            std::memcpy(msg.token, &buffer[idx], 32);
        }
        
        return msg;
    }
};
#pragma pack(pop)

// ListSensorRequestWithoutToken - Proxy envía al Storage (sin token)
// Tamaño: 1 byte (solo message_id)
#pragma pack(push, 1)
struct ListSensorRequestWithoutToken {
    uint8_t message_id = MSG_LIST_SENSOR_REQUEST;

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> result;
        result.push_back(message_id);
        return result;
    }

    static ListSensorRequestWithoutToken deserialize(const std::vector<uint8_t>& buffer) {
        ListSensorRequestWithoutToken msg;
        if (!buffer.empty()) {
            msg.message_id = buffer[0];
        }
        return msg;
    }
};
#pragma pack(pop)

// ListSensorResponse - Proxy responde con lista de sensores (ID 17)
// Tamaño variable: 2 bytes header + (16 bytes * sensorCount)
struct ListSensorResponse {
    uint8_t message_id = MSG_LIST_SENSOR_RESPONSE;
    uint8_t sensorCount;   // Número de sensores disponibles
    std::vector<std::array<char, 16>> sensorIds;  // Lista de IDs de sensores

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> result;
        result.push_back(message_id);
        result.push_back(sensorCount);
        
        // Serializar cada sensor ID (16 bytes cada uno)
        for (const auto& sensorId : sensorIds) {
            result.insert(result.end(), sensorId.begin(), sensorId.end());
        }
        
        return result;
    }

    static ListSensorResponse deserialize(const std::vector<uint8_t>& buffer) {
        ListSensorResponse msg;
        size_t idx = 0;
        
        msg.message_id = buffer[idx++];
        msg.sensorCount = buffer[idx++];
        
        // Deserializar cada sensor ID
        for (int i = 0; i < msg.sensorCount && idx + 16 <= buffer.size(); i++) {
            std::array<char, 16> sensorId;
            std::memcpy(sensorId.data(), &buffer[idx], 16);
            idx += 16;
            msg.sensorIds.push_back(sensorId);
        }
        
        return msg;
    }
};

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

struct UltrasonicSensorData {
    uint8_t message_id = MSG_ULTRASONIC_SENSOR_DATA;
    uint8_t sensor_id = ULTRASONIC_SENSOR;
    uint8_t date;
    uint8_t time;
    uint8_t data_lenght;
    uint16_t echo = 0;

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data;
        data.push_back(message_id);
        data.push_back(sensor_id);
        data.push_back(echo & 0xFF); // little-endian
        data.push_back((echo >> 8) & 0xFF);
        return data;
    }

    static UltrasonicSensorData deserialize(const std::vector<uint8_t>& buffer) {
        UltrasonicSensorData msg;
        if (!buffer.empty()) {
            msg.message_id = buffer[0];
            msg.sensor_id = buffer[1];
            msg.echo = buffer[2] | (buffer[3] << 8);
        }
        return msg;
    }
};

struct TiltSensorData {
    uint8_t message_id = MSG_TILT_SENSOR_DATA;
    uint8_t sensor_id = TILT_SENSOR;
    uint8_t date;
    uint8_t time;
    uint8_t data_lenght;
    uint16_t tilt = 0; // This sensor only sends 1 or 0

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data;
        data.push_back(message_id);
        data.push_back(sensor_id);
        data.push_back(tilt & 0xFF); // little-endian
        data.push_back((tilt >> 8) & 0xFF);
        return data;
    }

    static TiltSensorData deserialize(const std::vector<uint8_t>& buffer) {
        TiltSensorData msg;
        if (!buffer.empty()) {
            msg.message_id = buffer[0];
            msg.sensor_id = buffer[1];
            msg.tilt = buffer[2] | (buffer[3] << 8);
        }
        return msg;
    }
};

struct SoundSensorData {
    uint8_t message_id = MSG_SOUND_SENSOR_DATA;
    uint8_t sensor_id = SOUND_SENSOR;
    uint8_t date;
    uint8_t time;
    uint8_t data_lenght;
    uint16_t volume = 0;

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data;
        data.push_back(message_id);
        data.push_back(sensor_id);
        data.push_back(volume & 0xFF); // little-endian
        data.push_back((volume >> 8) & 0xFF);
        return data;
    }

    static SoundSensorData deserialize(const std::vector<uint8_t>& buffer) {
        SoundSensorData msg;
        if (!buffer.empty()) {
            msg.message_id = buffer[0];
            msg.sensor_id = buffer[1];
            msg.volume = buffer[2] | (buffer[3] << 8);
        }
        return msg;
    }
};

struct HumiditySensorData {
    uint8_t message_id = MSG_HUMIDITY_SENSOR_DATA;
    uint8_t sensor_id = HUMIDITY_SENSOR;
    uint8_t date;
    uint8_t time;
    uint8_t data_lenght;
    uint16_t temperature = 0;

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data;
        data.push_back(message_id);
        data.push_back(sensor_id);
        data.push_back(temperature & 0xFF); // little-endian
        data.push_back((temperature >> 8) & 0xFF);

        return data;
    }

    static HumiditySensorData deserialize(const std::vector<uint8_t>& buffer) {
        HumiditySensorData msg;
        if (!buffer.empty()) {
            msg.message_id = buffer[0];
            msg.sensor_id = buffer[1];
            msg.temperature = buffer[2] | (buffer[3] << 8);
        }
        return msg;
    }
};

struct SensorsData {
    uint16_t message_id = MSG_SENSORS_DATA;
    uint16_t echo = 0; // From the ultrasonic sensor
    uint16_t tilt = 0; // From the Tilt sensor
    uint16_t volume = 0; // From the sound sensor
    uint16_t temperature = 0; // From the humidity sensor

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> data;

        // message_id (big-endian: high byte first)
        data.push_back((message_id >> 8) & 0xFF);
        data.push_back(message_id & 0xFF);

        // echo (big-endian)
        data.push_back((echo >> 8) & 0xFF);
        data.push_back(echo & 0xFF);

        // volume (big-endian)
        data.push_back((volume >> 8) & 0xFF);
        data.push_back(volume & 0xFF);

        // temperature (big-endian)
        data.push_back((temperature >> 8) & 0xFF);
        data.push_back(temperature & 0xFF);

        return data;
    }

    static SensorsData deserialize(const std::vector<uint8_t>& buffer) {
        SensorsData msg;
        if (!buffer.empty()) {
            // little-endian)
            msg.message_id = (static_cast<uint16_t>(buffer[1]) << 8)
                             | static_cast<uint16_t>(buffer[0]);

            msg.echo = (static_cast<uint16_t>(buffer[3]) << 8)
                       | static_cast<uint16_t>(buffer[2]);

            msg.tilt = (static_cast<uint16_t>(buffer[5]) << 8)
                       | static_cast<uint16_t>(buffer[4]);

        }
        return msg;
    }
};
