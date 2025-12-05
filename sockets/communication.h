#pragma once
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <vector>
#include "crypto.h"

// Crear socket servidor
inline int create_server(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) return -1;
    if (listen(sock, 6) < 0) return -1;
    return sock;
}

// Crear socket cliente
inline int connect_to(const std::string& ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        return -1;
    return sock;
}

// Enviar mensaje binario (cifrado)
inline bool send_message(int sock, const void* data, size_t size) {
    std::vector<uint8_t> encrypted(size);
    crypto::encrypt(data, encrypted.data(), size);
    ssize_t sent = send(sock, encrypted.data(), size, 0);
    return sent == (ssize_t)size;
}

// Recibir mensaje binario (descifrado)
inline ssize_t recv_message(int sock, void* buffer, size_t size) {
    ssize_t received = recv(sock, buffer, size, 0);
    if (received > 0) {
        crypto::xor_transform(buffer, received);
        std::cout << "Mensaje encriptado: " << received << " bytes" << std::endl;
    }
    return received;
}
