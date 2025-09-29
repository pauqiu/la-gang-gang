#include "encryptation.h"
#include "security.h"

#include <vector>

#define USERS_PATH "Users.txt"
#define MAX_USERS_DATA 3072

#include <QDebug>

Security::Security(FileSystem * storage): storage(storage) {}

int Security::verifyUser(QString username, QString password)
{
    std::vector<char> users = storage->readFile(USERS_PATH);

    // TODO (@Paulette): check if user exist in the file.

    /*for (int i=0; i<users.size(); i++) {
        qDebug() << users[i];
    }*/

    return 0;
}

/**
 * The character ':' is the chosen separator for user info.
 **/

int Security::registerUser(QString username, QString password, QString role)
{
    // TODO (@Paulette): Verify that password and username are valid.
    std::string hashed_pwd = Encryptation::encryptPassword(password);
    std::string user_info = username.toStdString() + ":" + hashed_pwd + ":"
                            + role.toStdString() + "\n";

    storage->appendToFile(USERS_PATH, user_info);
    return 0;
}

bool Security::validPassword()
{

}
