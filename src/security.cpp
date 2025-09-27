#include "security.h"

Security::Security(FileSystem * storage): storage(storage) {}

int Security::verifyUser(QString username, QString password)
{
    return 0;
}

int Security::registerUser(QString username, QString password)
{
    return 0;
}
