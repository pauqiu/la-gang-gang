#pragma once
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>

// Tipo de mensaje
enum MessageType : uint8_t {
    MSG_AUTHENTICATION = 1,
    MSG_AUTH_RESPONSE  = 2,
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
