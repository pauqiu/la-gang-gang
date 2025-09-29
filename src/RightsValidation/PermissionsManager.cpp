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
  std::cout << "DEBUG: Current permissions for role " << roleId << ": '" << currentPermissions << "'" << std::endl;

  std::vector<std::string> currentPerms = parsePermissions(currentPermissions);
  std::vector<std::string> newPerms = parsePermissions(permissions);
  std::string mergedPermissions = mergePermissions(currentPerms, newPerms);

  std::cout << "DEBUG: Merged permissions: '" << mergedPermissions << "'" << std::endl;

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
  std::cout << "Removing permissions from role " << roleId << ": "
            << permissions << std::endl;
  std::cout << "Remove permissions functionality not yet implemented."
            << std::endl;
  return true;
}

std::string PermissionsManager::getPermissions(int roleId) {
  if (!roleExists(roleId)) {
    std::cerr << "Error: Role with ID " << roleId << " does not exist."
              << std::endl;
    return "";
  }

  RolesFileManager fileManager(fs);
  std::vector<std::string> lines = fileManager.readRolesFile();

  std::cout << "DEBUG: Searching for role " << roleId << " in " << lines.size() << " lines" << std::endl;

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
        std::cout << "DEBUG: Found ID: " << currentId << std::endl;
        
        if (currentId == roleId) {
          // Ahora obtener el nombre del rol y los permisos
          if (std::getline(iss, roleName, ';')) {
            std::getline(iss, permissions); // Puede estar vacío
            std::cout << "DEBUG: Found role " << roleId << " - Permissions: '" << permissions << "'" << std::endl;
            return permissions;
          }
        }
      } catch (const std::exception &e) {
        std::cerr << "DEBUG: Error parsing ID from: " << idStr << " - " << e.what() << std::endl;
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

  std::cout << "DEBUG: Updating permissions for role " << roleId << " to: " << newPermissions << std::endl;

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
          // Obtener el nombre del rol
          if (std::getline(iss, roleName, ';')) {
            std::getline(iss, currentPermissions); // Permisos actuales
            
            std::string newLine = buildRoleLine(idStr, roleName, newPermissions);
            std::cout << "DEBUG: Updating line from: " << line << std::endl;
            std::cout << "DEBUG: Updating line to: " << newLine << std::endl;
            
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
    std::cout << "DEBUG: Writing updated file with " << lines.size() << " lines" << std::endl;
    return fileManager.writeRolesFile(lines);
  } else {
    std::cerr << "DEBUG: Could not find role " << roleId << " to update" << std::endl;
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