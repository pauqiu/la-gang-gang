#include "encryptation.h"
#include "security.h"

#include <QDebug>

Security::Security(FileSystem * storage): storage(storage) {}

int Security::verifyUser(QString username, QString password)
{
    return 0;
}

/**
 * The character ':' is the chosen separator for user info.
 **/

int Security::registerUser(QString username, QString password)
{
    // TODO: Verify that password and username are valid.
    std::string hashed_pwd = Encryptation::encryptPassword(password);
    std::string user_info = username.toStdString() + ":" + hashed_pwd;
    storage->writeFile("Users.txt", user_info);
    return 0;
}
