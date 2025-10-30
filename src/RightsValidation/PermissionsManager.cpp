#include "../../include/RightsValidation/PermissionsManager.h"
#include "../../include/RightsValidation/RoleManager.h"
#include "../../include/RightsValidation/RolesFileManager.h"
#include <algorithm>
#include <iostream>
#include <sstream>

PermissionsManager::PermissionsManager(FileSystem *filesystem)
    : fs(filesystem) {}

bool PermissionsManager::roleExists(int roleId) {
  RoleManager roleManager(fs);
  return roleManager.roleExists(roleId);
}

bool PermissionsManager::addPermissions(int roleId,
                                        const std::string &permissions) {
  if (!roleExists(roleId)) {
    std::cerr << "Error: Role with ID " << roleId << " does not exist."
              << std::endl;
    return false;
  }

  if (permissions.empty()) {
    std::cerr << "Error: Permissions cannot be empty." << std::endl;
    return false;
  }

  std::string currentPermissions = getPermissions(roleId);

  std::vector<std::string> currentPerms = parsePermissions(currentPermissions);
  std::vector<std::string> newPerms = parsePermissions(permissions);
  std::string mergedPermissions = mergePermissions(currentPerms, newPerms);

  if (updateRolePermissions(roleId, mergedPermissions)) {
    std::cout << "Successfully added permissions to role " << roleId << ": "
              << permissions << std::endl;
    std::cout << "Updated permissions: " << mergedPermissions << std::endl;
    return true;
  } else {
    std::cerr << "Error: Failed to update permissions in file." << std::endl;
    return false;
  }
}

bool PermissionsManager::removePermissions(int roleId,
                                           const std::string &permissions) {
  if (!roleExists(roleId)) {
    std::cerr << "Error: Role with ID " << roleId << " does not exist."
              << std::endl;
    return false;
  }

  if (permissions.empty()) {
    std::cerr << "Error: Permissions to remove cannot be empty." << std::endl;
    return false;
  }

  std::string currentPermissions = getPermissions(roleId);
  if (currentPermissions.empty()) {
    std::cout << "Role " << roleId << " has no permissions to remove."
              << std::endl;
    return true;
  }

  std::vector<std::string> currentPerms = parsePermissions(currentPermissions);
  std::vector<std::string> permsToRemove = parsePermissions(permissions);

  if (currentPerms.empty()) {
    std::cout << "Role " << roleId << " has no permissions to remove."
              << std::endl;
    return true;
  }

  std::vector<std::string> newPerms;
  for (const auto &perm : currentPerms) {
    if (std::find(permsToRemove.begin(), permsToRemove.end(), perm) ==
        permsToRemove.end()) {
      newPerms.push_back(perm);
    }
  }

  std::string updatedPermissions;
  for (size_t i = 0; i < newPerms.size(); ++i) {
    updatedPermissions += newPerms[i];
    if (i < newPerms.size() - 1) {
      updatedPermissions += ",";
    }
  }

  // Update file
  if (updateRolePermissions(roleId, updatedPermissions)) {
    std::cout << "Successfully removed permissions from role " << roleId << ": "
              << permissions << std::endl;
    std::cout << "Remaining permissions: " << updatedPermissions << std::endl;
    return true;
  } else {
    std::cerr << "Error: Failed to update permissions in file." << std::endl;
    return false;
  }
}

std::string PermissionsManager::getPermissions(int roleId) {
  if (!roleExists(roleId)) {
    std::cerr << "Error: Role with ID " << roleId << " does not exist."
              << std::endl;
    return "";
  }

  RolesFileManager fileManager(fs);
  std::vector<std::string> lines = fileManager.readRolesFile();

  for (const auto &line : lines) {
    // Saltar líneas de comentario
    if (line.empty() || line[0] == '#') {
      continue;
    }

    std::istringstream iss(line);
    std::string idStr, roleName, permissions;

    // Primero obtener el ID
    if (std::getline(iss, idStr, ';')) {
      try {
        int currentId = std::stoi(idStr);

        if (currentId == roleId) {
          // Get role name and permissions
          if (std::getline(iss, roleName, ';')) {
            std::getline(iss, permissions);
            return permissions;
          }
        }
      } catch (const std::exception &e) {
        continue;
      }
    }
  }

  return "";
}

bool PermissionsManager::updateRolePermissions(
    int roleId, const std::string &newPermissions) {
  RolesFileManager fileManager(fs);
  std::vector<std::string> lines = fileManager.readRolesFile();
  bool updated = false;

  for (auto &line : lines) {
    if (line.empty() || line[0] == '#') {
      continue;
    }

    std::istringstream iss(line);
    std::string idStr, roleName, currentPermissions;

    if (std::getline(iss, idStr, ';')) {
      try {
        int currentId = std::stoi(idStr);
        if (currentId == roleId) {
          // Get role name and permissions
          if (std::getline(iss, roleName, ';')) {
            std::getline(iss, currentPermissions);

            std::string newLine =
                buildRoleLine(idStr, roleName, newPermissions);
            line = newLine;
            updated = true;
            break;
          }
        }
      } catch (const std::exception &e) {
        continue;
      }
    }
  }

  if (updated) {
    return fileManager.writeRolesFile(lines);
  }

  return false;
}

std::vector<std::string>
PermissionsManager::parsePermissions(const std::string &permissionsStr) {
  std::vector<std::string> permissions;
  if (permissionsStr.empty()) {
    return permissions;
  }

  std::istringstream iss(permissionsStr);
  std::string perm;

  while (std::getline(iss, perm, ',')) {
    perm.erase(0, perm.find_first_not_of(" \t"));
    perm.erase(perm.find_last_not_of(" \t") + 1);

    if (!perm.empty()) {
      permissions.push_back(perm);
    }
  }

  return permissions;
}

std::string PermissionsManager::mergePermissions(
    const std::vector<std::string> &currentPerms,
    const std::vector<std::string> &newPerms) {
  std::vector<std::string> mergedPerms = currentPerms;

  for (const auto &newPerm : newPerms) {
    if (std::find(mergedPerms.begin(), mergedPerms.end(), newPerm) ==
        mergedPerms.end()) {
      mergedPerms.push_back(newPerm);
    }
  }

  std::string result;
  for (size_t i = 0; i < mergedPerms.size(); ++i) {
    result += mergedPerms[i];
    if (i < mergedPerms.size() - 1) {
      result += ",";
    }
  }

  return result;
}

std::string PermissionsManager::buildRoleLine(const std::string &idStr,
                                              const std::string &roleName,
                                              const std::string &permissions) {
  return idStr + ";" + roleName + ";" + permissions;
}