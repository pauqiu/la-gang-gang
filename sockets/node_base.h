#pragma once
#include "communication.h"
#include "message_dispatcher.h"
#include <thread>
#include <atomic>
#include <vector>

class NodeBase {
public:
    NodeBase(int port) : port(port), running(false) {}

    void start() {
        running = true;
        serverThread = std::thread(&NodeBase::serverLoop, this);
    }

    void stop() {
        running = false;
        if (serverThread.joinable()) serverThread.join();
    }

    void sendTo(const std::string& ip, int port, const std::vector<uint8_t>& data) {
        int sock = connect_to(ip, port);
        if (sock < 0) {
            std::cerr << "Error conectando a " << ip << ":" << port << std::endl;
            return;
        }
        send_message(sock, data.data(), data.size());
        close(sock);
    }

protected:
    MessageDispatcher dispatcher;
    int port;
    std::atomic<bool> running;
    std::thread serverThread;

public:

    void serverLoop() {
        int server = create_server(port);
        if (server < 0) {
            std::cerr << "Error iniciando servidor en puerto " << port << std::endl;
            return;
        }
        std::cout << "[Node] Escuchando en puerto " << port << "...\n";

        while (running) {
            sockaddr_in client_addr{};
            socklen_t size = sizeof(client_addr);
            int client = accept(server, (sockaddr*)&client_addr, &size);
            if (client < 0) continue;

            std::vector<uint8_t> buffer(1024);
            ssize_t bytes = recv_message(client, buffer.data(), buffer.size());
            if (bytes <= 0) {
                std::cerr << "[Node] conexión cerrada o error\n";
                close(client);
                continue;
            }
            buffer.resize(bytes);
            std::cout << "[Node] Recibidos " << bytes << " bytes, id=" << (int)buffer[0] << "\n";
            dispatcher.dispatch(buffer);

            close(client);
        }

        close(server);
    }
};
