#pragma once
#include "communication.h"
#include "messages.h"
#include <iostream>
#include <vector>

class NodeClient {
public:
    // Enviar mensaje de autenticación
    void sendAuthentication(const std::string& user, const std::string& pass, int tries = 0) {
        AuthMessage msg;
        msg.user = user;
        msg.password = pass;
        msg.failedTries = tries;

        // Serializar el mensaje
        std::vector<uint8_t> data = msg.serialize();

        // Conectarse al servidor Auth
        int sock = connect_to("127.0.0.1", 5001);
        if (sock < 0) {
            std::cerr << "[Client] Error al conectar con Auth.\n";
            return;
        }

        // Enviar mensaje binario
        if (send_message(sock, data.data(), data.size())) {
            std::cout << "[Client] Mensaje de autenticación enviado.\n";
        } else {
            std::cerr << "[Client] Error al enviar mensaje.\n";
        }

        close(sock);
    }
};
