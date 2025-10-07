#include "node_base.h"
#include "messages.h"
#include <iostream>

class NodeAuth : public NodeBase {
public:
    NodeAuth(int port) : NodeBase(port) {
        // Registrar handlers
        dispatcher.registerHandler(MSG_AUTHENTICATION, 
            [this](const std::vector<uint8_t>& buf) { onAuthentication(buf); });
    }

    void onAuthentication(const std::vector<uint8_t>& buf) {
        auto msg = AuthMessage::deserialize(buf);
        std::cout << "[AuthNode] Usuario: " << msg.user 
                  << ", Password: " << msg.password << std::endl;

        std::vector<uint8_t> reply = { MSG_AUTH_RESPONSE, 1 };
        //sendTo("127.0.0.1", 5002, reply); // ejemplo: enviar respuesta a otro nodo
    }
};
