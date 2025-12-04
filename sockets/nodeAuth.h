#include "node_base.h"
#include "messages.h"
#include "security.h"
#include "filesystem.h"
#include "RightsValidation/RightsValidation.h"
#include "logger.h"
#include "endpoints.h"
#include <iostream>
#include <cstring>
#include <random>

class NodeAuth : public NodeBase {
public:
    NodeAuth(int port, Security* securityInstance, RightsValidation* rightsInstance, FileSystem* fs)
        : NodeBase(port), security(securityInstance), rights(rightsInstance), logger(fs, "aLogs.bin") {
        setupLogSupport(fs, "aLogs.bin", NODE_AUTH);
        
        dispatcher.registerHandler(MSG_AUTHENTICATION,
                                   [this](const std::vector<uint8_t>& buf, int client_socket) {
                                       onAuthentication(buf, client_socket);
                                   });

        dispatcher.registerHandler(MSG_LOG_REQUEST,
                                   [this](const std::vector<uint8_t>& buf, int client_socket) {
                                       onLogRequest(buf, client_socket);
                                   });

        // Handlers para gestión de usuarios/roles
        dispatcher.registerHandler(MSG_USER_CREATE,
                                   [this](const std::vector<uint8_t>& buf, int client_socket) {
                                       onUserCreate(buf, client_socket);
                                   });

        dispatcher.registerHandler(MSG_USER_UPDATE,
                                   [this](const std::vector<uint8_t>& buf, int client_socket) {
                                       onUserUpdate(buf, client_socket);
                                   });

        dispatcher.registerHandler(MSG_ROLE_CREATE,
                                   [this](const std::vector<uint8_t>& buf, int client_socket) {
                                       onRoleCreate(buf, client_socket);
                                   });

        dispatcher.registerHandler(MSG_ROLE_UPDATE,
                                   [this](const std::vector<uint8_t>& buf, int client_socket) {
                                       onRoleUpdate(buf, client_socket);
                                   });

        dispatcher.registerHandler(MSG_USERS_LIST_REQUEST,
                                   [this](const std::vector<uint8_t>& buf, int client_socket) {
                                       onUsersListRequest(buf, client_socket);
                                   });

        dispatcher.registerHandler(MSG_ROLES_LIST_REQUEST,
                                   [this](const std::vector<uint8_t>& buf, int client_socket) {
                                       onRolesListRequest(buf, client_socket);
                                   });
    }

    void onAuthentication(const std::vector<uint8_t>& buf, int client_socket) {
        auto msg = AuthMessage::deserialize(buf);
        std::cout << "[AuthNode] Usuario: " << msg.user << ", Password: " << msg.password << "\n";

        // Validar credenciales usando Security
        bool credentialsValid = false;
        uint8_t userRole = 0;
        uint8_t errorCode = 0;
        validateCredentials(msg, credentialsValid, userRole, errorCode);

        // Enviar respuesta
        if (credentialsValid) {
            sendSuccessResponse(client_socket, msg.user, userRole);
            logger.success("Autenticación exitosa - Usuario: " + msg.user +
                           " | Rol: " + std::to_string((int)userRole));
        } else {
            sendErrorResponse(client_socket, errorCode);
            logger.warning("Intento de autenticación fallido - Usuario: " + msg.user +
                           " | Error code: " + std::to_string((int)errorCode));
        }

        close(client_socket);
    }

    // --- Handlers para gestión de usuarios/roles ---
    void onUserCreate(const std::vector<uint8_t>& buf, int client_socket) {
        size_t i = 1;
        if (i >= buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        uint8_t ulen = buf[i++];
        if (i + ulen > buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        std::string username((char*)&buf[i], ulen); i += ulen;
        if (i >= buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        uint8_t plen = buf[i++];
        if (i + plen > buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        std::string password((char*)&buf[i], plen); i += plen;
        if (i >= buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        uint8_t rlen = buf[i++];
        if (i + rlen > buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        std::string role((char*)&buf[i], rlen); i += rlen;

        bool ok = (security->registerUser(QString::fromStdString(username), QString::fromStdString(password), QString::fromStdString(role)) == 0);
        sendManageResponse(client_socket, ok ? 0 : 1);
        close(client_socket);
    }

    void onUserUpdate(const std::vector<uint8_t>& buf, int client_socket) {
        size_t i = 1;
        if (i >= buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        uint8_t oldlen = buf[i++];
        if (i + oldlen > buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        std::string oldUser((char*)&buf[i], oldlen); i += oldlen;
        if (i >= buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        uint8_t newlen = buf[i++];
        if (i + newlen > buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        std::string newUser((char*)&buf[i], newlen); i += newlen;
        if (i >= buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        uint8_t rlen = buf[i++];
        if (i + rlen > buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        std::string newRole((char*)&buf[i], rlen); i += rlen;

        int res = security->updateUser(QString::fromStdString(oldUser), QString::fromStdString(newUser), QString::fromStdString(newRole));
        sendManageResponse(client_socket, res == 0 ? 0 : 1);
        close(client_socket);
    }

    void onRoleCreate(const std::vector<uint8_t>& buf, int client_socket) {
        size_t i = 1;
        if (i + 4 > buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        int id = 0;
        for (int b = 0; b < 4; ++b) id = (id << 8) | buf[i++];
        if (i >= buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        uint8_t rlen = buf[i++];
        if (i + rlen > buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        std::string roleName((char*)&buf[i], rlen); i += rlen;
        if (i >= buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        uint8_t plen = buf[i++];
        if (i + plen > buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        std::string permissions((char*)&buf[i], plen); i += plen;

        bool ok = false;
        if (rights) {
            ok = rights->addRole(id, roleName);
            if (ok && !permissions.empty()) ok = rights->addPermissions(id, permissions);
        }
        sendManageResponse(client_socket, ok ? 0 : 1);
        close(client_socket);
    }

    void onRoleUpdate(const std::vector<uint8_t>& buf, int client_socket) {
        size_t i = 1;
        if (i >= buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        uint8_t oldlen = buf[i++];
        if (i + oldlen > buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        std::string oldRole((char*)&buf[i], oldlen); i += oldlen;
        if (i >= buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        uint8_t newlen = buf[i++];
        if (i + newlen > buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        std::string newRole((char*)&buf[i], newlen); i += newlen;
        if (i >= buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        uint8_t plen = buf[i++];
        if (i + plen > buf.size()) { sendManageResponse(client_socket, 1); close(client_socket); return; }
        std::string permissions((char*)&buf[i], plen); i += plen;

        bool ok = false;
        if (rights) {
            ok = rights->getRoleManager().updateRole(oldRole, newRole, permissions);
        }
        sendManageResponse(client_socket, ok ? 0 : 1);
        close(client_socket);
    }

    void onUsersListRequest(const std::vector<uint8_t>& buf, int client_socket) {
        auto users = security->getUsers();
        std::vector<uint8_t> out;
        out.push_back(MSG_USERS_LIST_RESPONSE);
        out.push_back((uint8_t)users.size());
        for (const auto &u : users) {
            std::string uname = u[0];
            std::string role = u[2];
            out.push_back((uint8_t)std::min<size_t>(uname.size(), 255));
            out.insert(out.end(), uname.begin(), uname.end());
            out.push_back((uint8_t)std::min<size_t>(role.size(), 255));
            out.insert(out.end(), role.begin(), role.end());
        }
        send_message(client_socket, out.data(), out.size());
        close(client_socket);
    }

    void onRolesListRequest(const std::vector<uint8_t>& buf, int client_socket) {
        std::vector<std::string> lines = rights->getRoleManager().readRolesFile();
        std::vector<uint8_t> out;
        out.push_back(MSG_ROLES_LIST_RESPONSE);
        out.push_back((uint8_t)lines.size());
        for (const auto &line : lines) {
            out.push_back((uint8_t)std::min<size_t>(line.size(), 255));
            out.insert(out.end(), line.begin(), line.end());
        }
        send_message(client_socket, out.data(), out.size());
        close(client_socket);
    }

    void sendManageResponse(int client_socket, uint8_t status) {
        ManageResponse r; r.message_id = MSG_MANAGE_RESPONSE; r.status = status;
        auto data = r.serialize();
        send_message(client_socket, data.data(), data.size());
    }

private:
    Security* security;  // Puntero a Security
    RightsValidation* rights;
    Logger logger;


    void validateCredentials(const AuthMessage& msg, bool& valid,
                             uint8_t& role, uint8_t& errorCode) {
        QString username = QString::fromStdString(msg.user);
        QString password = QString::fromStdString(msg.password);

        // Usar Security::verifyUser - ya retorna el número de rol
        int result = security->verifyUser(username, password);

        if (result >= 0) {
            valid = true;

            role = static_cast<uint8_t>(result);  // verifyUser ya retorna 1-7

            QString roleStr = security->getUserRole(username);  // Solo para logging

            std::string logMsg = "Credenciales validadas - Usuario: " + msg.user +
                                 " | Rol: " + roleStr.toStdString() +
                                 " (" + std::to_string((int)role) + ")";
            logger.info(logMsg);
        } else {
            valid = false;
            errorCode = 1;
            logger.warning("Validación fallida - Usuario: " + msg.user);
        }
    }

    void sendSuccessResponse(int client_socket, const std::string& username, uint8_t role) {
        AuthResponse response;
        response.message_id = MSG_AUTH_RESPONSE;
        response.role = role;
        generateSessionToken(response.token);

        // Enviar respuesta al cliente
        auto data = response.serialize();
        send_message(client_socket, data.data(), data.size());
        std::cout << "[AuthNode] AuthResponse enviado al cliente (role=" << (int)role << ")\n";

        // Registrar token en el proxy
        registerTokenWithProxy(username, response.token, role);
    }

    void registerTokenWithProxy(const std::string& username, const uint8_t token[32], uint8_t role) {
        TokenNotif tokenMsg;
        tokenMsg.message_id = MSG_TOKEN_REGISTER;
        std::memcpy(tokenMsg.token, token, 32);

        // Copiar username (máximo 16 bytes)
        std::memset(tokenMsg.username, 0, 16);
        size_t len = std::min(username.length(), size_t(16));
        std::memcpy(tokenMsg.username, username.c_str(), len);

        tokenMsg.role = role;

        // Enviar token al proxy (puerto 5002)
        auto data = tokenMsg.serialize();
        sendTo(getProxyIp(), getProxyPort(), data);
        std::cout << "[AuthNode] TokenNotif enviado a Proxy (user=" << username << ", role=" << (int)role << ")\n";
    }

    void sendErrorResponse(int client_socket, uint8_t errorCode) {
        AuthError error;
        error.message_id = MSG_AUTH_ERROR;
        error.error_code = errorCode;

        auto data = error.serialize();
        send_message(client_socket, data.data(), data.size());
        std::cout << "[AuthNode] AuthError enviado (error_code=" << (int)errorCode << ")\n";
    }

    void generateSessionToken(uint8_t token[32]) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 255);
        for (int i = 0; i < 32; i++) {
            token[i] = static_cast<uint8_t>(dis(gen));
        }
    }
    
    // Handler para solicitud de logs
    void onLogRequest(const std::vector<uint8_t>& buf, int client_socket) {
        auto clientMsg = LogRequest::deserialize(buf);
        
        // logger.info("LogRequest recibido - Node Type: " + std::to_string(clientMsg.node_type));
        
        // Auth no valida tokens (es el que los genera), procesa directamente
        if (clientMsg.node_type == NODE_AUTH) {
            std::vector<std::string> logs = getLogsInRange(clientMsg.startDate, clientMsg.endDate);
            sendLogResponse(client_socket, NODE_AUTH, logs);
            // logger.success("Logs enviados al cliente: " + std::to_string(logs.size()) + " entradas");
        } else {
            logger.warning("Solicitud de logs para otro nodo recibida en Auth");
            // sendLogResponse(client_socket, clientMsg.node_type, {});
        }
        
        close(client_socket);
    }
};
