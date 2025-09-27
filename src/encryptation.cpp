#include <sodium.h>

#include "encryptation.h"


Encryptation::Encryptation() {}

/**
*   Important note: Libsodium is a C library.
*   It handles C parameters, so it doesn't
*   support variables like strings.
**/

std::string Encryptation::encryptPassword(QString password)
{
    // Converting QString to const * char
    std::string stdString = password.toStdString();
    const char* cPassword = stdString.c_str();

    char hashed_password[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str(hashed_password, cPassword, strlen(cPassword),
                          crypto_pwhash_OPSLIMIT_MODERATE, crypto_pwhash_MEMLIMIT_MODERATE) != 0) {
        // Hashing failed
        return "";
    }

    return std::string(hashed_password);
}

int Encryptation::decryptPassword()
{
    return 0;
}
