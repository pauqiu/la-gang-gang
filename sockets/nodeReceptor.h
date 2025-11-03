#ifndef NODERECEPTOR_H
#define NODERECEPTOR_H

#include "node_base.h"
#include "messages.h"
#include "filesystem.h"

#include <string>
#include <ctime>

class nodeReceptor:public NodeBase {
public:

    nodeReceptor(int port, FileSystem * fileSystem)
        : NodeBase(port), fileSystem(fileSystem) {

        dispatcher.registerHandler(MSG_SENSORS_DATA,
                                   [this](const std::vector<uint8_t>& buf, int client_socket) {
                                       onSensorsReceive(buf, client_socket);
                                   });
    }

private:
    FileSystem * fileSystem;

    // TODO(@Paulette): Implement logs for this node.
    void onSensorsReceive(const std::vector<uint8_t>& buf, int client_socket) {
        auto msg = SensorsData::deserialize(buf);

        std::cout << "[ReceptorNode] Guardando datos - Volumen: " << msg.volume
                  << ", Echo: " << msg.echo << ", Temperatura: " << msg.temperature
                  << "\n";

        //std::string output = std::string("[ReceptorNode] Guardando datos - Volumen: ") + std::to_string(msg[1]) +
        //                                    ", Echo: " + std::to_string(msg[2]) +
        //                                    ", Temperatura: " + std::to_string(msg[3]) + "\n";

        //std::cout << output << "\n";

        // uint8_t volume = std::stoi(msg[1]);
        // uint8_t echo = std::stoi(msg[2]);
        // uint8_t temperature = std::stoi(msg[3]);
        
        // Log goes here
        //newlogEntry(output);

        // Prepare data to send
        time_t now = time(nullptr);
        uint8_t date = static_cast<uint64_t>(now / 86400ULL);
        uint8_t time = static_cast<uint64_t>(now % 86400ULL);

        // Send data to storage
        close(client_socket);
    }

    void newlogEntry(std::string log) {
        fileSystem->appendToFile("ReceptorLogs.bin", log);
    }
};

#endif // NODERECEPTOR_H
