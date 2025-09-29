#include "../../include/RightsValidation/RightsValidation.h"
#include "../../include/RightsValidation/PermissionsManager.h"
#include "../../include/RightsValidation/RoleManager.h"
#include "../../include/RightsValidation/RolesFileManager.h"
#include <iostream>

RightsValidation::RightsValidation(FileSystem *filesystem)
    : fs(filesystem), roleManager(filesystem), permissionsManager(filesystem) {}

bool RightsValidation::initialize() {
  std::cout << "\n=== Initializing Rights Validation System ===" << std::endl;

  // Inicializar el archivo de roles
  RolesFileManager fileManager(fs);
  bool success = fileManager.createRolesFile();

  if (success) {
    std::cout << "Rights validation system initialized successfully."
              << std::endl;
  } else {
    std::cerr << "Failed to initialize rights validation system." << std::endl;
  }
  
  return success;
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