#include "../../include/RightsValidation/RightsValidation.h"
#include "../../include/RightsValidation/PermissionsManager.h"
#include "../../include/RightsValidation/RoleManager.h"
#include "../../include/RightsValidation/RolesFileManager.h"
#include <iostream>

RightsValidation::RightsValidation(FileSystem *filesystem)
    : fs(filesystem), roleManager(filesystem), permissionsManager(filesystem) {}

bool RightsValidation::initialize() {
  std::cout << "\nInitializing Rights Validation System" << std::endl;

   RolesFileManager fileManager(fs);
  if (!fileManager.createRolesFile()) {
    std::cerr << "Error: Failed to create or access roles file." << std::endl;
    return false;
  }

  std::cout << "Sucesss: Rights validation system initialized successfully."
            << std::endl;
  return true;
}

// Métodos delegados a RoleManager
bool RightsValidation::roleExists(int roleId) {
  return roleManager.roleExists(roleId);
}

bool RightsValidation::addRole(int id, const std::string &roleName) {
  return roleManager.addRole(id, roleName);
}

bool RightsValidation::removeRole(int roleId) {
  return roleManager.removeRole(roleId);
}

// Métodos delegados a PermissionsManager
bool RightsValidation::addPermissions(int roleId,
                                      const std::string &permissions) {
  return permissionsManager.addPermissions(roleId, permissions);
}

bool RightsValidation::removePermissions(int roleId,
                                         const std::string &permissions) {
  return permissionsManager.removePermissions(roleId, permissions);
}

std::string RightsValidation::getPermissions(int roleId) {
  return permissionsManager.getPermissions(roleId);
}