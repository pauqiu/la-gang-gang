#pragma once

#include <string>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "messages.h"
#include "communication.h"

/**
 * HealthChecker - Verifica si un nodo está activo
 */
class HealthChecker {
public:
    static constexpr int DEFAULT_TIMEOUT_MS = 500;

    /**
     * Verifica si un nodo responde al health check
     */
    static bool isNodeAlive(const std::string& ip, int port, int timeoutMs = DEFAULT_TIMEOUT_MS) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) return false;

        // Configurar timeout
        struct timeval tv;
        tv.tv_sec = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs % 1000) * 1000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        // Conectar
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

        if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
            ::close(sock);
            return false;
        }

        // Enviar health check (cifrado)
        HealthCheck check;
        auto data = check.serialize();
        if (!send_message(sock, data.data(), data.size())) {
            ::close(sock);
            return false;
        }

        // Esperar respuesta (descifrado)
        std::vector<uint8_t> buffer(8);
        ssize_t received = recv_message(sock, buffer.data(), buffer.size());
        ::close(sock);

        return (received > 0 && buffer[0] == MSG_HEALTH_RESPONSE);
    }
};
