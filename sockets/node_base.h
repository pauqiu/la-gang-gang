#pragma once
#include "communication.h"
#include "message_dispatcher.h"
#include "messages.h"
#include "filesystem.h"
#include <thread>
#include <atomic>
#include <vector>
#include <sstream>
#include <algorithm>

class NodeBase {
public:
    NodeBase(int port) : port(port), running(false), nodeFilesystem(nullptr), nodeType(0) {
        dispatcher.registerHandler(MSG_HEALTH_CHECK,
            [this](const std::vector<uint8_t>&, int client_socket) {
                HealthResponse resp;
                resp.node_type = nodeType;
                auto data = resp.serialize();
                send_message(client_socket, data.data(), data.size());
                close(client_socket);
            });
    }

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
    
    // Para funcionalidad de logs
    FileSystem* nodeFilesystem;
    std::string logFileName;
    uint8_t nodeType;
    
    // Configurar el nodo para soporte de logs
    void setupLogSupport(FileSystem* fs, const std::string& logFile, uint8_t type) {
        nodeFilesystem = fs;
        logFileName = logFile;
        nodeType = type;
    }
    
    // Obtener logs en un rango de fechas desde el archivo de logs del nodo
    std::vector<std::string> getLogsInRange(uint64_t startDate, uint64_t endDate) {
        std::vector<std::string> result;
        
        if (!nodeFilesystem || logFileName.empty()) return result;
        
        std::vector<char> content = nodeFilesystem->readFile(logFileName);
        if (content.empty()) return result;
        
        std::string fileContent(content.begin(), content.end());
        std::istringstream iss(fileContent);
        std::string line;
        
        while (std::getline(iss, line)) {
            if (line.empty()) continue;
            if (line.length() < 10) continue;
            
            // Check format YYYY-MM-DD
            if (line[4] != '-' || line[7] != '-') continue;
            
            try {
                std::string dateStr = line.substr(0, 10);
                dateStr.erase(std::remove(dateStr.begin(), dateStr.end(), '-'), dateStr.end());
                
                uint64_t date = std::stoull(dateStr);
                
                if (date >= startDate && date <= endDate) {
                    result.push_back(line);
                }
            } catch (...) {
                continue;
            }
        }
        
        return result;
    }
    
    // Enviar respuesta de logs al cliente
    void sendLogResponse(int client_socket, uint8_t type, const std::vector<std::string>& logs) {
        LogResponse response;
        response.message_id = MSG_LOG_RESPONSE;
        response.node_type = type;
        response.logCount = logs.size();
        response.logs = logs;
        
        auto data = response.serialize();
        send_message(client_socket, data.data(), data.size());
    }

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
            
            dispatcher.dispatch(buffer, client);
            
            // Ya no se cierra el socket
        }

        close(server);
    }
};
