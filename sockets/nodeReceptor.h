#ifndef NODERECEPTOR_H
#define NODERECEPTOR_H

#include "node_base.h"
#include "messages.h"
#include "filesystem.h"
#include "logger.h"
#include "endpoints.h"

#include <string>
#include <ctime>

class nodeReceptor:public NodeBase {
public:

    nodeReceptor(int port, FileSystem * fileSystem)
        : NodeBase(port), logger(fileSystem, "rLogs.bin") {

        dispatcher.registerHandler(MSG_SENSORS_DATA,
                                   [this](const std::vector<uint8_t>& buf, 
                                        int client_socket) {
                                       onSensorsReceive(buf, client_socket);
                                   });
    }

private:
    Logger logger;

    void onSensorsReceive(const std::vector<uint8_t>& buf, int client_socket) {
        auto msg = SensorsData::deserialize(buf);

        std::cout << "[ReceptorNode] Guardando datos - Tilt: " << msg.tilt
                  << ", Echo: " << msg.echo << ", Volume: " << msg.volume
                  << ", Intensidad de la luz: " << msg.light_intensity
                  << "\n";

        // Prepare data to send
        time_t now = time(nullptr);
        
        struct tm *tm_info = localtime(&now);
        uint64_t date = (tm_info->tm_year + 1900) * 10000ULL
                        + (tm_info->tm_mon + 1) * 100ULL
                        + tm_info->tm_mday;

        uint64_t time_val = tm_info->tm_hour * 10000ULL
                  + tm_info->tm_min  * 100ULL
                  + tm_info->tm_sec;

        // Logs for received data
        logger.info("Received data from tilt sensor: " + 
                    std::to_string(msg.tilt));

        logger.info("Received data from ultrasonic sensor: " + 
                    std::to_string(msg.echo));

        logger.info("Received data from sound sensor: " + 
                    std::to_string(msg.volume));

        logger.info("Received data from light sensor: " + 
                    std::to_string(msg.light_intensity));

        // TODO(@Paulette): Log data received from the rest of the sensors
        
        sendToSTorage(date, time_val, msg.tilt, TILT_SENSOR);
        sendToSTorage(date, time_val, msg.echo, ULTRASONIC_SENSOR);
        sendToSTorage(date, time_val, msg.volume, SOUND_SENSOR);
        sendToSTorage(date, time_val, msg.light_intensity, LIGHT_SENSOR);

        // Send data to storage
        close(client_socket);
    }

    void sendToSTorage(uint64_t date, uint64_t time, uint16_t value, 
                        uint8_t sensor_id) {
        // TODO(@Paulette): Grab this from the endpoints.txt
        int sock = connect_to("10.1.35.15", 5004);
        if (sock < 0) {
            std::cerr << "[ReceptorNode] Error conectando a Storage\n";
        }

        uint8_t alert_signal = alert(value, sensor_id);

        // Empaquetar datos: [value(2bytes)][status(1byte)]
        std::vector<uint8_t> data;
        data.push_back((value >> 8) & 0xFF);
        data.push_back(value & 0xFF);
        data.push_back(alert_signal);

        StorageSave msg;
        msg.sensorId = sensor_id;
        msg.date = date;
        msg.time = time;
        msg.data = data;
        msg.dataLength = data.size();

        auto serialized = msg.serialize();

        std::string alert_value = (alert_signal == 1) ? "ALERT" : "NORMAL";

        std::cout << "[ReceptorNode] Enviando: " << date << " " << time
                    << " | Value: " << value
                    << " | Status: " << alert_value << "\n";

        if (!send_message(sock, serialized.data(), serialized.size())) {
            std::cerr << "[ReceptorNode] Error enviando mensaje\n";
            logger.error("Couldn't send data to Storage.");
            close(sock);
        }

        logger.success("Sent data to Storage succesfully.");
    }

    uint8_t alert(uint16_t value, uint8_t sensor) {

        uint8_t alert_signal = 0;

        switch (sensor) 
        {
        case ULTRASONIC_SENSOR:
            if (value < 100) {
                alert_signal = 1;
            }
            break;

        case TILT_SENSOR:
            if (value == 1) {
                alert_signal = 1;   
            }
            break;

        case SOUND_SENSOR:
            if(value > 25) {
                alert_signal = 1;
            }
            break;

        case LIGHT_SENSOR:
            if(value < 300) {
                alert_signal = 1;
            }
            break;

        default:
            alert_signal = -1;
            break;
        }

        return alert_signal;
    }
};

#endif // NODERECEPTOR_H
