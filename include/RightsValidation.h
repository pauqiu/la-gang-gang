#ifndef RIGHTS_VALIDATION_H
#define RIGHTS_VALIDATION_H

#include <string>
#include "filesystem.h"

class RightsValidation {
private:
    FileSystem* fs;
    std::string rolesFileName = "roles.txt";
    
    bool createRolesFile();

public:
    RightsValidation(FileSystem* filesystem);
    bool initialize();
    bool roleExists(int roleId);
    bool addRole(int id, const std::string& roleName);
    bool addPermissions(int roleId, const std::string& permissions);
    std::string getPermissions(int roleId);
    bool removeRole(int roleId);
    bool removePermissions(int roleId, const std::string& permissions);
};

#endif