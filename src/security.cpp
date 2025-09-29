#include "encryptation.h"
#include "security.h"

#include <iostream>
#include <sstream>
#include <vector>

#define USERS_PATH "Users.txt"
#define MAX_USERS_DATA 3072

#include <QDebug>

Security::Security(FileSystem * storage): storage(storage) {}

int Security::verifyUser(QString username, QString password)
{
    std::vector<char> users = storage->readFile(USERS_PATH);

    // TODO (@Paulette): check if user exist in the file.

    std::string currentUser;

    for (int ch = 0; ch < users.size(); ch++) {
        if (users[ch] == ':') {
            std::vector<std::string> user = splitUserInfo(currentUser);
            for (std::string element : user) {
                std::cout << element << " ";
            }
            currentUser.clear();
        } else {
            currentUser += users[ch];
        }
    }

    return 0;
}

/**
 * The character ':' is the chosen separator for user info.
 **/

int Security::registerUser(QString username, QString password, QString role)
{
    // TODO (@Paulette): Verify that password and username are valid.
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
