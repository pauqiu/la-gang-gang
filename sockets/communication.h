#pragma once
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

// ----------------------------------------------------
// Crear socket servidor
// ----------------------------------------------------
int create_server(int port) {
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

// ----------------------------------------------------
// Crear socket cliente
// ----------------------------------------------------
int connect_to(const std::string& ip, int port) {
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

// ----------------------------------------------------
// Enviar mensaje binario
// ----------------------------------------------------
bool send_message(int sock, const void* data, size_t size) {
    ssize_t sent = send(sock, data, size, 0);
    return sent == (ssize_t)size;
}

// ----------------------------------------------------
// Recibir mensaje binario
// ----------------------------------------------------
ssize_t recv_message(int sock, void* buffer, size_t size) {
    return recv(sock, buffer, size, 0);
}
