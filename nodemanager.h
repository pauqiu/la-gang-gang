#pragma once
#include <string>
#include <array>
#include <iostream>
#include <unistd.h>
#include <libssh/libssh.h>
#include "endpoints.h"
#include "communication.h"

// Estructura para información de cada nodo
struct NodeInfo {
    std::string name;
    std::string ip;
    int port;
    std::string sshUser;
    std::string sshPassword;
    std::string startCommand;
    std::string stopCommand;
};

class NodeManager {
public:
    inline static const std::string SSH_USER = "lab-3-5";
    inline static const std::string SSH_PASSWORD = "Cata2960!";
    inline static const std::string BASE_PATH = "~/Documents/la-gang-gang/build/bin";
    inline static const std::string AUTH_EXEC = "nodeAuthMain";
    inline static const std::string PROXY_EXEC = "proxyMain";
    inline static const std::string STORAGE_EXEC = "nodeStorageMain";
    inline static const std::string RECEPTOR_EXEC = "nodeReceptor";

    NodeManager() {
        loadEndpoints("endpoints.txt");

        authNode = {
            "Auth",
            getAuthIp(),
            getAuthPort(),
            SSH_USER,
            SSH_PASSWORD,
            "cd " + BASE_PATH + " && nohup ./" + AUTH_EXEC + " > /dev/null 2>&1 &",
            "pkill -f " + AUTH_EXEC
        };

        proxyNode = {
            "Proxy",
            getProxyIp(),
            getProxyPort(),
            SSH_USER,
            SSH_PASSWORD,
            "cd " + BASE_PATH + " && nohup ./" + PROXY_EXEC + " > /dev/null 2>&1 &",
            "pkill -f " + PROXY_EXEC
        };

        storageNode = {
            "Storage",
            getStorageIp(),
            getStoragePort(),
            SSH_USER,
            SSH_PASSWORD,
            "cd " + BASE_PATH + " && nohup ./" + STORAGE_EXEC + " > /dev/null 2>&1 &",
            "pkill -f " + STORAGE_EXEC
        };

        receptorNode = {
            "Receptor",
            getReceptorIp(),
            getReceptorPort(),
            SSH_USER,
            SSH_PASSWORD,
            "cd " + BASE_PATH + " && nohup ./" + RECEPTOR_EXEC + " > /dev/null 2>&1 &",
            "pkill -f " + RECEPTOR_EXEC
        };
    }

    bool checkNodeStatus(const std::string& ip, int port);
    std::array<bool, 4> checkAllNodes();
    void checkSingleNode(const NodeInfo& node);
    void startNode(const NodeInfo& node);
    void stopNode(const NodeInfo& node);
    int executeSSHCommand(const NodeInfo& node, const std::string& command);

    void startAuth()    { startNode(authNode); }
    void stopAuth()     { stopNode(authNode); }
    void startProxy()   { startNode(proxyNode); }
    void stopProxy()    { stopNode(proxyNode); }
    void startStorage() { startNode(storageNode); }
    void stopStorage()  { stopNode(storageNode); }
    void startReceptor(){ startNode(receptorNode); }
    void stopReceptor() { stopNode(receptorNode); }

private:
    NodeInfo authNode, proxyNode, storageNode, receptorNode;
};
