#ifndef ENCRYPTATION_H
#define ENCRYPTATION_H

#include <QString>
#include <string>

class Encryptation {

public:
    Encryptation();
    ~Encryptation() = default;
    static std::string encryptPassword(QString password);
    int decryptPassword();
};

#endif // ENCRYPTATION_H
