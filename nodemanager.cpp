#include "nodemanager.h"

bool NodeManager::checkNodeStatus(const std::string& ip, int port) {
    int sock = connect_to(ip, port);
    if (sock < 0) {
        return false;
    } else {
        close(sock);
        return true;
    }
}

int NodeManager::executeSSHCommand(const NodeInfo& node, const std::string& command) {
    ssh_session session = ssh_new();
    if (session == nullptr) {
        std::cerr << "[SSH Error] No se pudo crear sesión SSH.\n";
        return -1;
    }

    ssh_options_set(session, SSH_OPTIONS_HOST, node.ip.c_str());
    ssh_options_set(session, SSH_OPTIONS_USER, node.sshUser.c_str());
    int sshPort = 22;
    ssh_options_set(session, SSH_OPTIONS_PORT, &sshPort);

    // Conectar
    int rc = ssh_connect(session);
    if (rc != SSH_OK) {
        std::cerr << "[SSH Error] Error al conectar a " << node.ip
                  << ": " << ssh_get_error(session) << "\n";
        ssh_free(session);
        return -1;
    }

    // Autenticar
    rc = ssh_userauth_password(session, nullptr, node.sshPassword.c_str());
    if (rc != SSH_AUTH_SUCCESS) {
        std::cerr << "[SSH Error] Autenticación fallida en " << node.ip
                  << ": " << ssh_get_error(session) << "\n";
        ssh_disconnect(session);
        ssh_free(session);
        return -1;
    }

    // Ejecutar comando
    ssh_channel channel = ssh_channel_new(session);
    if (channel == nullptr) {
        std::cerr << "[SSH Error] No se pudo crear canal SSH.\n";
        ssh_disconnect(session);
        ssh_free(session);
        return -1;
    }

    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        std::cerr << "[SSH Error] No se pudo abrir sesión del canal.\n";
        ssh_channel_free(channel);
        ssh_disconnect(session);
        ssh_free(session);
        return -1;
    }

    rc = ssh_channel_request_exec(channel, command.c_str());
    if (rc != SSH_OK) {
        std::cerr << "[SSH Error] Error ejecutando comando: " << ssh_get_error(session) << "\n";
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        ssh_disconnect(session);
        ssh_free(session);
        return -1;
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    ssh_disconnect(session);
    ssh_free(session);

    return 0;
}

void NodeManager::startNode(const NodeInfo& node) {
    std::cout << "[Info] Iniciando nodo " << node.name << " en " << node.ip << "...\n";

    if (checkNodeStatus(node.ip, node.port)) {
        return;
    }

    int result = executeSSHCommand(node, node.startCommand);

    if (result == 0) {
        sleep(2);
        std::cout << "Comando ejecutado" << "...\n";

        if (checkNodeStatus(node.ip, node.port)) {
            std::cout << "[OK] Nodo " << node.name << " iniciado correctamente.\n";
        } else {
            std::cout << "[Advertencia] Comando enviado pero el nodo no responde aún.\n";
        }
    }
}

void NodeManager::stopNode(const NodeInfo& node) {
    std::cout << "[Info] Deteniendo nodo " << node.name << " en " << node.ip << "...\n";

    if (!checkNodeStatus(node.ip, node.port)) {
        return;
    }

    int result = executeSSHCommand(node, node.stopCommand);

    if (result == 0) {
        sleep(1);

        if (!checkNodeStatus(node.ip, node.port)) {
            std::cout << "[OK] Nodo " << node.name << " detenido correctamente.\n";
        } else {
            std::cout << "[Advertencia] El nodo aún responde.\n";
        }
    }
}

std::array<bool, 4> NodeManager::checkAllNodes() {
    std::array<bool, 4> states;
    states[0] = checkNodeStatus(authNode.ip, authNode.port);      // Auth
    states[1] = checkNodeStatus(proxyNode.ip, proxyNode.port);    // Proxy
    states[2] = checkNodeStatus(storageNode.ip, storageNode.port); // Storage
    states[3] = checkNodeStatus(receptorNode.ip, receptorNode.port); // Receptor
    return states;
}


void NodeManager::checkSingleNode(const NodeInfo& node) {
    std::cout << "Nodo: " << node.name << " (" << node.ip << ":" << node.port << ") - ";

    if (checkNodeStatus(node.ip, node.port)) {
        std::cout << "✓ ENCENDIDO\n";
    } else {
        std::cout << "✗ APAGADO\n";
    }
}
