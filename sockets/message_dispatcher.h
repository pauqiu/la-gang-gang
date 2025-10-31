#pragma once
#include <map>
#include <functional>
#include <vector>
#include <cstdint>
#include <iostream>

class MessageDispatcher {
public:
    using Handler = std::function<void(const std::vector<uint8_t>&, int)>;

    void registerHandler(uint8_t messageId, Handler handler) {
        handlers[messageId] = handler;
    }

    void registerHandlerArduino(uint8_t messageId, Handler handler) {
        std::string id_string = std::to_string(static_cast<int>(messageId));
        handlersArduinos[id_string] = handler;
    }

    void dispatch(const std::vector<uint8_t>& buffer, int client_socket) {
        if (buffer.empty()) return;
        uint8_t id = buffer[0];
        if (handlers.count(id))
            handlers[id](buffer, client_socket);
        else
            std::cerr << "[WARN] Mensaje desconocido ID=" << (int)id << std::endl;
    }


    void dispatchReceptor(const std::vector<std::string>& buffer, int client_socket) {
        if (buffer.empty()) return;

        // Assuming the first string represents an ID (as before)
        uint8_t id = static_cast<uint8_t>(std::stoi(buffer[0]));

        if (handlersArduinos.count(id))
            handlersArduinos[id](buffer, client_socket);
        else
            std::cerr << "[WARN] Mensaje desconocido ID=" << (int)id << std::endl;
    }

private:
    std::map<uint8_t, Handler> handlers;
    std::map<std::string, Handler> handlersArduinos;
};
