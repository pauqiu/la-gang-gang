#include "encryptation.h"
#include "security.h"

#include <sstream>
#include <vector>

#define USERS_PATH "Users.txt"
#define MAX_USERS_DATA 3072

#include <QDebug>

/**
 *  User's file format:
 *
 *  username:hashed password:role
 *
 **/

Security::Security(FileSystem * storage): storage(storage) {}

int Security::verifyUser(QString username, QString password)
{
    std::vector<std::string> user = getUser(username);

    if (user.empty()) {
        qDebug() << "User not found";
        return -1;
    }

    if(!Encryptation::veifyPassword(password, user[1])) {
        qDebug() << "The password is incorrect";
        return -1;
    }

    // TODO (@Paulette): Return role to the main window.

    return 0;
}

int Security::registerUser(QString username, QString password, QString role)
{
    // TODO (@Paulette): Make this validation more solid.
    if (!validPassword(password) || !validUser(username)) return 1;

    std::string hashed_pwd = Encryptation::encryptPassword(password);
    std::string user_info = username.toStdString() + ":" + hashed_pwd + ":"
                            + role.toStdString() + "\n";

    storage->appendToFile(USERS_PATH, user_info);
    return 0;
}

bool Security::validPassword(QString password)
{
    if (password.size() > 10 || password.contains(":")) {
        qDebug() << "Invalid password";
        return false;
    }

    return true;
}

bool Security::validUser(QString password)
{
    if (password.size() > 16 || password.contains(":")) {
        qDebug() << "Invalid user";
        return false;
    }

    return true;
}

std::vector<std::string> Security::getUser(QString username)
{
    std::vector<char> users = storage->readFile(USERS_PATH);

    std::vector<std::string> user;
    std::string currentUser;

    for (int ch = 0; ch < users.size(); ch++) {
        if (users[ch] == '\n') {
            user = splitUserInfo(currentUser);
            if (user[0] == username.toStdString()) {
                qDebug() << "User " << user[0] << " found";
                break;
            }
            user.clear();
            currentUser.clear();
        } else {
            currentUser += users[ch];
        }
    }
    return user;
}

std::vector<std::string> Security::splitUserInfo(const std::string userInfo)
{
    std::vector<std::string> result;
    std::stringstream auxiliar(userInfo);
    std::string token;

    while(std::getline(auxiliar, token, ':')) {
        result.push_back(token);
    }

    return result;
}
