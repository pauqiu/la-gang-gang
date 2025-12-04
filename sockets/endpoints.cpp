#include "endpoints.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <map>

// Almacenamiento de endpoints
static std::string authIp, proxyIp, storageIp, storage2Ip, receptorIp;
static int authPort = 0, proxyPort = 0, storagePort = 0, storage2Port = 0, receptorPort = 0;
static bool loaded = false;
static bool storage2Available = false;
static bool receptorAvailable = false;

// Trim whitespace
static std::string trim(const std::string& s) {
    auto start = std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); });
    auto end = std::find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !std::isspace(c); }).base();
    return (start < end) ? std::string(start, end) : "";
}

// Parsear valor "ip:puerto"
static void parseAddress(const std::string& value, std::string& ip, int& port) {
    auto colonPos = value.find(':');
    if (colonPos == std::string::npos)
        throw std::runtime_error("Formato inválido (falta ':'): " + value);
    
    ip = trim(value.substr(0, colonPos));
    std::string portStr = trim(value.substr(colonPos + 1));
    
    if (ip.empty())
        throw std::runtime_error("IP vacía en: " + value);
    
    try {
        port = std::stoi(portStr);
        if (port <= 0 || port > 65535)
            throw std::runtime_error("Puerto fuera de rango: " + portStr);
    } catch (const std::invalid_argument&) {
        throw std::runtime_error("Puerto inválido: " + portStr);
    }
}

void loadEndpoints(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("No se pudo abrir: " + path);
    
    std::map<std::string, std::string> endpoints;
    std::string line;
    int lineNum = 0;
    
    while (std::getline(file, line)) {
        lineNum++;
        
        // Quitar CR si existe (Windows)
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        
        line = trim(line);
        
        // Ignorar líneas vacías y comentarios
        if (line.empty() || line[0] == '#')
            continue;
        
        // Buscar '='
        auto eqPos = line.find('=');
        if (eqPos == std::string::npos)
            throw std::runtime_error("Línea " + std::to_string(lineNum) + ": falta '=' en: " + line);
        
        std::string key = trim(line.substr(0, eqPos));
        std::string value = trim(line.substr(eqPos + 1));
        
        if (key.empty() || value.empty())
            throw std::runtime_error("Línea " + std::to_string(lineNum) + ": clave o valor vacío");
        
        endpoints[key] = value;
    }
    
    // Validar endpoints requeridos
    if (endpoints.find("AUTH") == endpoints.end())
        throw std::runtime_error("Falta endpoint AUTH");
    if (endpoints.find("PROXY") == endpoints.end())
        throw std::runtime_error("Falta endpoint PROXY");
    if (endpoints.find("STORAGE") == endpoints.end())
        throw std::runtime_error("Falta endpoint STORAGE");
    
    // Parsear endpoints requeridos
    parseAddress(endpoints["AUTH"], authIp, authPort);
    parseAddress(endpoints["PROXY"], proxyIp, proxyPort);
    parseAddress(endpoints["STORAGE"], storageIp, storagePort);
    
    // Parsear STORAGE2 si existe (opcional)
    if (endpoints.find("STORAGE2") != endpoints.end()) {
        parseAddress(endpoints["STORAGE2"], storage2Ip, storage2Port);
        storage2Available = true;
    }
    
    // Parsear RECEPTOR si existe (opcional)
    if (endpoints.find("RECEPTOR") != endpoints.end()) {
        parseAddress(endpoints["RECEPTOR"], receptorIp, receptorPort);
        receptorAvailable = true;
    }
    
    loaded = true;
}

// Helpers para verificar carga
static void ensureLoaded() {
    if (!loaded) throw std::runtime_error("Endpoints no cargados. Llame loadEndpoints() primero.");
}

// Getters - Auth
std::string getAuthIp()    { ensureLoaded(); return authIp; }
int         getAuthPort()  { ensureLoaded(); return authPort; }

// Getters - Proxy
std::string getProxyIp()   { ensureLoaded(); return proxyIp; }
int         getProxyPort() { ensureLoaded(); return proxyPort; }

// Getters - Storage (primario)
std::string getStorageIp()   { ensureLoaded(); return storageIp; }
int         getStoragePort() { ensureLoaded(); return storagePort; }

// Getters - Storage2 (secundario, opcional)
bool        hasStorage2()     { ensureLoaded(); return storage2Available; }
std::string getStorage2Ip()   { ensureLoaded(); return storage2Ip; }
int         getStorage2Port() { ensureLoaded(); return storage2Port; }

// Getters - Receptor (opcional)
bool        hasReceptor()     { ensureLoaded(); return receptorAvailable; }
std::string getReceptorIp()   { ensureLoaded(); return receptorIp; }
int         getReceptorPort() { ensureLoaded(); return receptorPort; }
