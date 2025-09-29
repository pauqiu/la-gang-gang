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

Security::Security(FileSystem * storage): storage(storage) {

    loadUsersList();
}

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
    std::vector<std::string> newUser;

    // TODO (@Paulette): Make this validation more solid.
    if (!validPassword(password) || !validUser(username)) return 1;

    std::string hashed_pwd = Encryptation::encryptPassword(password);
    std::string user_info = username.toStdString() + ":" + hashed_pwd + ":"
                            + role.toStdString() + "\n";

    newUser.push_back(username.toStdString());
    newUser.push_back(hashed_pwd);
    newUser.push_back(role.toStdString());

    this->registeredUsers.push_back(newUser);
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
    std::vector<std::string> user;

    for (int usr = 0; usr < this->registeredUsers.size();
         usr++) {
        if (this->registeredUsers[usr][0] == username.toStdString()) {
            user = registeredUsers[usr];
            break;
        }
    }

    return user;
}

void Security::loadUsersList()
{
    std::vector<char> users = storage->readFile(USERS_PATH);

    std::vector<std::vector<std::string>> usersList;
    std::vector<std::string> user;
    std::string currentUser;

    for (int ch = 0; ch < users.size(); ch++) {
        if (users[ch] == '\n') {
            user = splitUserInfo(currentUser);
            usersList.push_back(user);
            currentUser.clear();
        } else {
            currentUser += users[ch];
        }
    }
    this->registeredUsers = usersList;
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
