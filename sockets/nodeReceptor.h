#ifndef NODERECEPTOR_H
#define NODERECEPTOR_H

#include "node_base.h"
#include "nodeStorage.h"
#include "filesystem"

#include <string>

class nodeReceptor:NodeBase {
    nodeReceptor(int port, FileSystem * fileSystem)
        : NodeBase(port), fileSystem(fileSystem) {

        // TODO(@Paulette): Register handler by message.
        dispatcher.registerHandlerArduino(MSG_SENSORS_DATA,
                                   [this](const std::vector<std::string>& buf, int client_socket) {
                                       onSensorsReceive(buf, client_socket);
                                   });
    }



private:
    FileSystem * fileSystem;

    // TODO(@Paulette): Implement log for this node.
    void onSensorsReceive(const std::vector<std::string>& buf, int client_socket) {
        auto msg = buf

        std::cout << "[ReceptorNode] Guardando datos - Volumen: " << msg[1]
                  << ", Echo: " << msg[2] << ", Temperatura: " << msg[3] 
                  << "\n";

        
        // LOG

        // PREPARE DATA
        
        // SEND TO STORAGE
        close(client_socket);
    }

};

#endif // NODERECEPTOR_H
