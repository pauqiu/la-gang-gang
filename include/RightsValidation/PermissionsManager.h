#ifndef PERMISSIONSMANAGER_H
#define PERMISSIONSMANAGER_H

#include "filesystem.h"
#include <string>
#include <vector>

class PermissionsManager {
public:
    PermissionsManager(FileSystem *filesystem);
    bool addPermissions(int roleId, const std::string &permissions);
    bool removePermissions(int roleId, const std::string &permissions);
    std::string getPermissions(int roleId);
    bool updateRolePermissions(int roleId, const std::string &newPermissions);

private:
    FileSystem *fs;
    bool roleExists(int roleId);
    std::vector<std::string> parsePermissions(const std::string &permissionsStr);
    std::string mergePermissions(const std::vector<std::string> &currentPerms, const std::vector<std::string> &newPerms);
    std::string buildRoleLine(const std::string &idStr, const std::string &roleName, const std::string &permissions);
};

#endif