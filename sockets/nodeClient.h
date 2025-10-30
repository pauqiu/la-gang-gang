#pragma once
#include "communication.h"
#include "messages.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include <cstring>

class NodeClient {
public:
    bool sendAuthentication(const std::string& user, const std::string& pass, int tries = 0) {
        currentUsername = user; // Guardar username para validación posterior
        
        int sock = connectToAuthServer();
        if (sock < 0) return false;

        if (!sendAuthRequest(sock, user, pass, tries)) {
            close(sock);
            return false;
        }

        auto response = receiveAuthResponse(sock);
        if (!response.empty()) {
            processAuthResponse(response);
        }

        close(sock);
        return true;
    }
    
    bool validateSessionWithProxy() {
        if (!hasValidToken()) {
            std::cerr << "[Client] No hay token válido.\n";
            return false;
        }
        
        int sock = connectToProxy();
        if (sock < 0) return false;
        
        if (!sendSessionValidate(sock)) {
            close(sock);
            return false;
        }
        
        auto response = receiveResponse(sock);
        bool isValid = false;
        if (!response.empty()) {
            isValid = processSessionResponse(response);
        }
        
        close(sock);
        return isValid;
    }
    
    bool hasValidToken() const {
        return tokenReceived;
    }
    
    uint8_t getRole() const {
        return currentRole;
    }
    
    // Solicitar lista de sensores disponibles
    std::vector<std::string> requestSensorList() {
        if (!hasValidToken()) {
            std::cerr << "[Client] No hay token válido.\n";
            return {};
        }
        
        int sock = connectToProxy();
        if (sock < 0) return {};
        
        if (!sendListSensorRequest(sock)) {
            close(sock);
            return {};
        }
        
        auto response = receiveResponse(sock);
        std::vector<std::string> sensors;
        if (!response.empty()) {
            sensors = processListSensorResponse(response);
        }
        
        close(sock);
        return sensors;
    }
    
    // Solicitar datos de un sensor específico en un rango de fechas
    std::vector<SensorEntry> requestSensorData(const std::string& sensorId, 
                                                uint64_t startDate, 
                                                uint64_t endDate) {
        if (!hasValidToken()) {
            std::cerr << "[Client] No hay token válido. Autentíquese primero.\n";
            return {};
        }
        
        if (sensorId.empty() || sensorId.length() > 16) {
            std::cerr << "[Client] Sensor ID inválido.\n";
            return {};
        }
        
        int sock = connectToProxy();
        if (sock < 0) return {};
        
        if (!sendDataRequest(sock, sensorId, startDate, endDate)) {
            close(sock);
            return {};
        }
        
        auto response = receiveResponse(sock);
        std::vector<SensorEntry> entries;
        if (!response.empty()) {
            entries = processDataResponse(response);
        }

        close(sock);
        return entries;
    }

private:
    uint8_t sessionToken[32] = {0};
    uint8_t currentRole = 0;
    bool tokenReceived = false;
    std::string currentUsername;

    int connectToAuthServer() {
        int sock = connect_to("127.0.0.1", 5001);
        if (sock < 0) {
            std::cerr << "[Client] Error al conectar con Auth.\n";
        }
        return sock;
    }
    
    int connectToProxy() {
        int sock = connect_to("127.0.0.1", 5002);
        if (sock < 0) {
            std::cerr << "[Client] Error al conectar con Proxy.\n";
        }
        return sock;
    }

    bool sendAuthRequest(int sock, const std::string& user, const std::string& pass, int tries) {
        AuthMessage msg;
        msg.user = user;
        msg.password = pass;
        msg.failedTries = tries;

        auto data = msg.serialize();
        if (!send_message(sock, data.data(), data.size())) {
            std::cerr << "[Client] Error al enviar mensaje.\n";
            return false;
        }
        
        std::cout << "[Client] Mensaje de autenticación enviado. Esperando respuesta...\n";
        return true;
    }

    std::vector<uint8_t> receiveAuthResponse(int sock) {
        std::vector<uint8_t> response(1024);
        ssize_t bytes = recv_message(sock, response.data(), response.size());
        
        if (bytes <= 0) {
            std::cerr << "[Client] Error al recibir respuesta del servidor.\n";
            return {};
        }
        
        response.resize(bytes);
        std::cout << "[Client] Recibidos " << bytes << " bytes, ID: " << (int)response[0] << "\n";
        return response;
    }

    void processAuthResponse(const std::vector<uint8_t>& response) {
        if (response[0] == MSG_AUTH_RESPONSE) {
            handleAuthResponse(response);
        } else if (response[0] == MSG_AUTH_ERROR) {
            handleAuthError(response);
        } else {
            std::cerr << "[Client] Respuesta desconocida con ID: " << (int)response[0] << "\n";
        }
    }

    void handleAuthResponse(const std::vector<uint8_t>& data) {
        auto response = AuthResponse::deserialize(data);
        
        // Guardar token y rol
        std::memcpy(sessionToken, response.token, 32);
        currentRole = response.role;
        tokenReceived = true;
        
        std::cout << "\nAUTENTICACIÓN EXITOSA\n";
        std::cout << "Rol asignado: " << (int)response.role << "\n";
        std::cout << "Token de sesión (hex): ";
        printToken(response.token);
        std::cout << "\n";
    }
    
    void handleAuthError(const std::vector<uint8_t>& data) {
        auto error = AuthError::deserialize(data);
        
        std::cout << "\nERROR DE AUTENTICACIÓN\n";
        std::cout << "Código de error: " << (int)error.error_code << " - ";
        std::cout << getErrorMessage(error.error_code) << "\n\n";
    }

    void printToken(const uint8_t token[32]) {
        for (int i = 0; i < 32; i++) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)token[i];
        }
        std::cout << std::dec;
    }

    std::string getErrorMessage(uint8_t errorCode) {
        switch (errorCode) {
            case 1: return "Credenciales incorrectas";
            case 2: return "Exceso de intentos de autenticación";
            case 3: return "Token inválido o expirado";
            default: return "Error desconocido";
        }
    }
    
    bool sendSessionValidate(int sock) {
        SessionValidate msg;
        msg.message_id = MSG_SESSION_VALIDATE;
        
        // Copiar username (máximo 16 bytes)
        std::memset(msg.username, 0, 16);
        size_t len = std::min(currentUsername.length(), size_t(16));
        std::memcpy(msg.username, currentUsername.c_str(), len);
        
        // Copiar token
        std::memcpy(msg.token, sessionToken, 32);
        
        auto data = msg.serialize();
        if (!send_message(sock, data.data(), data.size())) {
            std::cerr << "[Client] Error al enviar validación de sesión.\n";
            return false;
        }
        
        std::cout << "[Client] Validación de sesión enviada al Proxy (user=" << currentUsername << ")...\n";
        return true;
    }
    
    std::vector<uint8_t> receiveResponse(int sock) {
        std::vector<uint8_t> response(1024);
        ssize_t bytes = recv_message(sock, response.data(), response.size());
        
        if (bytes <= 0) {
            std::cerr << "[Client] Error al recibir respuesta.\n";
            return {};
        }
        
        response.resize(bytes);
        return response;
    }
    
    bool processSessionResponse(const std::vector<uint8_t>& response) {
        if (response[0] == MSG_SESSION_OK) {
            auto msg = SessionOk::deserialize(response);
            std::cout << "\nSESIÓN VÁLIDA\n";
            std::cout << "Acceso concedido con rol: " << (int)msg.role << "\n";
            return true;
        } else if (response[0] == MSG_SESSION_ERROR) {
            auto msg = SessionError::deserialize(response);
            std::cout << "\nSESIÓN INVÁLIDA\n";
            std::cout << "Error: " << getErrorMessage(msg.error_code) << "\n\n";
            return false;
        } else {
            std::cerr << "[Client] Respuesta desconocida del Proxy.\n";
            return false;
        }
    }
    
    // Enviar solicitud de lista de sensores
    bool sendListSensorRequest(int sock) {
        ListSensorRequest msg;
        msg.message_id = MSG_LIST_SENSOR_REQUEST;
        
        // Copiar token
        std::memcpy(msg.token, sessionToken, 32);
        
        auto data = msg.serialize();
        if (!send_message(sock, data.data(), data.size())) {
            std::cerr << "[Client] Error al enviar solicitud de lista de sensores.\n";
            return false;
        }
        
        std::cout << "[Client] Solicitud de lista de sensores enviada al Proxy...\n";
        return true;
    }
    
    // Enviar solicitud de datos de sensor específico
    bool sendDataRequest(int sock, const std::string& sensorId, 
                        uint64_t startDate, uint64_t endDate) {
        DataRequest msg;
        msg.message_id = MSG_DATA_REQUEST;
        
        // Copiar token
        std::memcpy(msg.token, sessionToken, 32);
        
        // Copiar sensor ID (máximo 16 bytes)
        std::memset(msg.sensor_id, 0, 16);
        size_t len = std::min(sensorId.length(), size_t(16));
        std::memcpy(msg.sensor_id, sensorId.c_str(), len);
        
        // Asignar fechas
        msg.startDate = startDate;
        msg.endDate = endDate;
        
        auto data = msg.serialize();
        if (!send_message(sock, data.data(), data.size())) {
            std::cerr << "[Client] Error al enviar solicitud de datos.\n";
            return false;
        }
        
        std::cout << "[Client] Solicitud de datos enviada (sensor=" << sensorId 
                  << ", startDate=" << startDate << ", endDate=" << endDate << ")...\n";
        return true;
    }
    
    // Procesar respuesta de lista de sensores
    std::vector<std::string> processListSensorResponse(const std::vector<uint8_t>& response) {
        std::vector<std::string> sensors;
        
        if (response[0] != MSG_LIST_SENSOR_RESPONSE) {
            std::cerr << "[Client] Respuesta inesperada (ID=" << (int)response[0] << ")\n";
            return sensors;
        }
        
        auto msg = ListSensorResponse::deserialize(response);
        
        std::cout << "\nLISTA DE SENSORES DISPONIBLES (" << (int)msg.sensorCount << "):\n";
        for (const auto& sensorId : msg.sensorIds) {
            std::string sensor(sensorId.data(), strnlen(sensorId.data(), 16));
            sensors.push_back(sensor);
            std::cout << "  - " << sensor << "\n";
        }
        std::cout << "\n";
        
        return sensors;
    }
    
    // Procesar respuesta de datos de sensor
    std::vector<SensorEntry> processDataResponse(const std::vector<uint8_t>& response) {
        if (response.empty()) {
            std::cerr << "[Client] Respuesta vacía.\n";
            return {};
        }

        if (response[0] != MSG_DATA_RESPONSE) {
            std::cerr << "[Client] Respuesta inesperada (ID=" << (int)response[0] << ")\n";
            return {};
        }

        auto msg = DataResponse::deserialize(response);
        // logDataResponse(msg); // loggear en consola tabla de datos      
        return msg.entries;
    }

    void logDataResponse(const DataResponse& msg) {
        std::cout << "\nDATOS DEL SENSOR (" << (int)msg.entriesCount << " entradas):\n";
        std::cout << std::string(80, '-') << "\n";
        std::cout << std::left << std::setfill(' ')
                  << std::setw(18) << "Timestamp" 
                  << std::setw(18) << "Sensor ID"
                  << std::setw(15) << "Data Value"
                  << std::setw(10) << "Status" << "\n";
        std::cout << std::string(80, '-') << "\n";

        for (const auto& entry : msg.entries) {
            std::string dateStr = std::to_string(entry.date);
            std::string timeStr = std::to_string(entry.time);

            while (timeStr.length() < 6) {
                timeStr = "0" + timeStr;
            }

            std::string timestamp = dateStr.substr(0, 4) + "-" +
                                   dateStr.substr(4, 2) + "-" +
                                   dateStr.substr(6, 2) + " " +
                                   timeStr.substr(0, 2) + ":" +
                                   timeStr.substr(2, 2) + ":" +
                                   timeStr.substr(4, 2);

            std::string sensorId(entry.sensor_id, strnlen(entry.sensor_id, 16));
            std::string status(entry.status, strnlen(entry.status, 8));

            std::cout << std::left << std::setfill(' ')
                      << std::setw(18) << timestamp
                      << std::setw(18) << sensorId
                      << std::setw(15) << entry.data_value
                      << std::setw(10) << status << "\n";
        }

        std::cout << std::string(80, '-') << "\n\n";
    }
};
