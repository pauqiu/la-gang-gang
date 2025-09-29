#pragma once
#include <QString>

#ifndef ROLE_H
#define ROLE_H

struct Role {
    QString name;
    QString description;

    Role(QString name, QString description){
        this->name = name;
        this->description = description;
    }
};

#endif // ROLE_H
