#ifndef SECURITY_H
#define SECURITY_H

#include <QString>

#include "filesystem.h"

class Security
{
public:
    Security(FileSystem * storage);
    ~Security();
    int verifyUser(QString username, QString password);
    int registerUser(QString username, QString password);

private:
    int saveUser();
    int getUserCredentials();
    int compareCredentials();
    FileSystem * storage;
};

#endif // SECURITY_H
