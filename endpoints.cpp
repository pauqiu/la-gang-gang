#include "endpoints.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

static std::string authIp, proxyIp, storageIp, receptorIp;
static int authPort = 0, proxyPort = 0, storagePort = 0, receptorPort = 0;
static bool loaded = false;

static inline std::string trim(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    [](unsigned char ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    return s;
}

static void parseLine(const std::string& line, std::string& ip, int& port) {
    auto pos = line.find(':');
    if (pos == std::string::npos)
        throw std::runtime_error("Formato inválido: " + line);

    ip = trim(line.substr(0, pos));
    std::string portStr = trim(line.substr(pos + 1));

    try {
        port = std::stoi(portStr);
    } catch (...) {
        throw std::runtime_error("Puerto inválido: " + portStr);
    }
}

void loadEndpoints(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("No se pudo abrir el archivo: " + path);

    std::string lines[4];
    for (int i = 0; i < 4; ++i) {
        if (!std::getline(file, lines[i]))
            throw std::runtime_error("El archivo debe contener 4 líneas (Auth, Proxy, Storage, Receptor)");
        if (!lines[i].empty() && lines[i].back() == '\r')
            lines[i].pop_back();
    }

    parseLine(lines[0], authIp, authPort);
    parseLine(lines[1], proxyIp, proxyPort);
    parseLine(lines[2], storageIp, storagePort);
    parseLine(lines[3], receptorIp, receptorPort);
    loaded = true;
}

// Getters
std::string getAuthIp()        { if (!loaded) throw std::runtime_error("Endpoints no cargados"); return authIp; }
int         getAuthPort()      { if (!loaded) throw std::runtime_error("Endpoints no cargados"); return authPort; }

std::string getProxyIp()       { if (!loaded) throw std::runtime_error("Endpoints no cargados"); return proxyIp; }
int         getProxyPort()     { if (!loaded) throw std::runtime_error("Endpoints no cargados"); return proxyPort; }

std::string getStorageIp()     { if (!loaded) throw std::runtime_error("Endpoints no cargados"); return storageIp; }
int         getStoragePort()   { if (!loaded) throw std::runtime_error("Endpoints no cargados"); return storagePort; }

std::string getReceptorIp()    { if (!loaded) throw std::runtime_error("Endpoints no cargados"); return receptorIp; }
int         getReceptorPort()  { if (!loaded) throw std::runtime_error("Endpoints no cargados"); return receptorPort; }
