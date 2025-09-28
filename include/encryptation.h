#ifndef ENCRYPTATION_H
#define ENCRYPTATION_H

#include <QString>
#include <string>

class Encryptation {

public:
    static std::string encryptPassword(QString password);
    static bool veifyPassword(const QString password, const std::string hash);
};

#endif // ENCRYPTATION_H
