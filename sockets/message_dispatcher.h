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

    void dispatch(const std::vector<uint8_t>& buffer, int client_socket) {
        if (buffer.empty()) return;
        uint8_t id = buffer[0];
        if (handlers.count(id))
            handlers[id](buffer, client_socket);
        else
            std::cerr << "[WARN] Mensaje desconocido ID=" << (int)id << std::endl;
    }

private:
    std::map<uint8_t, Handler> handlers;
};
