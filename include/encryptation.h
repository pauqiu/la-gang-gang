#ifndef ENCRYPTATION_H
#define ENCRYPTATION_H

#include <QString>
#include <string>

#define SALT_LEN 16

class Encryptation {

public:
    static std::string encryptPassword(QString password);
    int decryptPassword();
};

#endif // ENCRYPTATION_H
