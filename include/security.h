#ifndef SECURITY_H
#define SECURITY_H

#include <QString>
#include <vector>

#include "filesystem.h"

class Security
{

// Attributes
private:
    FileSystem * storage;
    std::vector<std::vector<std::string>> registeredUsers;

public:
    Security(FileSystem * storage);
    ~Security();
    int verifyUser(QString username, QString password);
    int registerUser(QString username, QString password, QString role = "User");
    std::vector<std::vector<std::string>> getUsers();

private:
    bool validPassword(QString password);
    bool validUser(QString password);
    std::vector<std::string> getUser(QString username);
    void loadUsersList();
    std::vector<std::string> splitUserInfo(const std::string userInfo);
};

#endif // SECURITY_H
