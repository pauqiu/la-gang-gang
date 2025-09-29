#ifndef SECURITY_H
#define SECURITY_H

#include <QString>

#include "filesystem.h"

class Security
{

// Attributes
private:
    FileSystem * storage;

public:
    Security(FileSystem * storage);
    ~Security();
    int verifyUser(QString username, QString password);
    int registerUser(QString username, QString password, QString role = "User");

private:
    bool validPassword(QString password);
    bool validUser(QString password);
};

#endif // SECURITY_H
