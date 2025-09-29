#ifndef RIGHTS_VALIDATION_H
#define RIGHTS_VALIDATION_H

#include "filesystem.h"
#include "RoleManager.h"
#include "PermissionsManager.h"
#include <string>

class RightsValidation {
private:
    FileSystem* fs;
    RoleManager roleManager;
    PermissionsManager permissionsManager;

public:
    RightsValidation(FileSystem* filesystem);
    bool initialize();
    
    // Métodos delegados a RoleManager
    bool roleExists(int roleId);
    bool addRole(int id, const std::string& roleName);
    bool removeRole(int roleId);
    
    // Métodos delegados a PermissionsManager
    bool addPermissions(int roleId, const std::string& permissions);
    bool removePermissions(int roleId, const std::string& permissions);
    std::string getPermissions(int roleId);
};

#endif