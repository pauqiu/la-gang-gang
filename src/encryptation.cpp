#include <sodium.h>

#include "encryptation.h"

#include <QDebug>

/**
*   Important note: Libsodium is a C library.
*   It handles C parameters, so it doesn't
*   support variables like strings.
**/

std::string Encryptation::encryptPassword(QString password)
{
    if (sodium_init() < 0) {
        qDebug() << "Library initialization failed";
        return "";
    }

    // Converting QString to a safe buffer
    QByteArray utf8 = password.toUtf8();
    const char * pwd = utf8.constData();
    size_t pwd_len = utf8.size();

    char hashed_password[crypto_pwhash_STRBYTES];

    /**
     * Note: crypto_pwhash_str generates the salt.
    **/
    if (crypto_pwhash_str(hashed_password, pwd, pwd_len,
                          crypto_pwhash_OPSLIMIT_MODERATE, crypto_pwhash_MEMLIMIT_MODERATE) != 0) {
        // Hashing failed
        return "";
    }

    return std::string(hashed_password);
}

bool Encryptation::veifyPassword(const QString password, const std::string hash)
{
    if (sodium_init() < 0) {
        qDebug() << "Library initialization failed";
        return false;
    }

    // Converting QString to a safe buffer
    QByteArray utf8 = password.toUtf8();
    const char * pwd = utf8.constData();
    size_t pwd_len = utf8.size();

    return crypto_pwhash_str_verify(hash.c_str(), pwd, pwd_len) == 0;
}

