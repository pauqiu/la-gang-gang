#pragma once
#include <string>
#include <map>
#include <vector>
#include <optional>

/**
 * SensorRegistry - Catálogo centralizado de sensores
 * 
 * Responsabilidades:
 * - Mantener el mapeo bidireccional entre IDs textuales y numéricos
 * - Proveer lista de sensores disponibles
 * - Validar existencia de sensores
 * 
 * Futuro: Esta clase puede extenderse para cargar sensores desde archivo/DB
 * o recibir actualizaciones desde la UI de administrador.
 */
class SensorRegistry {
public:
    static SensorRegistry& getInstance() {
        static SensorRegistry instance;
        return instance;
    }

    // Obtener ID numérico desde string
    std::optional<uint8_t> getNumericId(const std::string& sensorId) const {
        auto it = stringToNumeric.find(sensorId);
        if (it != stringToNumeric.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // Obtener ID string desde numérico
    std::optional<std::string> getStringId(uint8_t numericId) const {
        auto it = numericToString.find(numericId);
        if (it != numericToString.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // Verificar si un sensor existe (por string)
    bool exists(const std::string& sensorId) const {
        return stringToNumeric.find(sensorId) != stringToNumeric.end();
    }

    // Verificar si un sensor existe (por ID numérico)
    bool exists(uint8_t numericId) const {
        return numericToString.find(numericId) != numericToString.end();
    }

    // Obtener lista de todos los IDs string
    std::vector<std::string> getAllSensorIds() const {
        std::vector<std::string> ids;
        for (const auto& [id, _] : stringToNumeric) {
            ids.push_back(id);
        }
        return ids;
    }

    // Obtener cantidad de sensores registrados
    size_t count() const {
        return stringToNumeric.size();
    }

    // TODO: Métodos para agregar/eliminar sensores dinámicamente
    // void addSensor(const std::string& id, uint8_t numericId);
    // void removeSensor(const std::string& id);
    // void loadFromFile(const std::string& filename);
    // void saveToFile(const std::string& filename);

private:
    SensorRegistry() {
        // Catálogo inicial hardcodeado
        // En el futuro, esto puede cargarse desde archivo o base de datos
        registerSensor("PIR001", 1);
        registerSensor("DHT11A", 2);
        registerSensor("HC001", 3);
        registerSensor("VB001", 4);
    }

    void registerSensor(const std::string& stringId, uint8_t numericId) {
        stringToNumeric[stringId] = numericId;
        numericToString[numericId] = stringId;
    }

    std::map<std::string, uint8_t> stringToNumeric;
    std::map<uint8_t, std::string> numericToString;

    // Singleton: eliminar constructores de copia
    SensorRegistry(const SensorRegistry&) = delete;
    SensorRegistry& operator=(const SensorRegistry&) = delete;
};
