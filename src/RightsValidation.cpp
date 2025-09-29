#include "RightsValidation.h"
#include "filesystem.h"
#include <iostream>

RightsValidation::RightsValidation(FileSystem *filesystem) : fs(filesystem) {}

bool RightsValidation::createRolesFile() {
  // Verificar si el archivo roles.txt ya existe en el directorio actual (root)
  int fileInode =
      fs->findInDirectory(fs->getCurrentDirectoryInode(), rolesFileName);

  if (fileInode == -1) {
    // El archivo no existe, crearlo
    if (fs->createFile(rolesFileName)) {
      std::cout << "File '" << rolesFileName << "' created successfully."
                << std::endl;

      // Crear archivo con estructura básica
      std::string content = "# Roles System File\n";
      content += "# Format: id;role_name;permissions\n";

      fs->overwriteFile(rolesFileName, content);
      std::cout << "Basic structure added to '" << rolesFileName << "'."
                << std::endl;
      return true;
    } else {
      std::cerr << "Error: Could not create file '" << rolesFileName << "'."
                << std::endl;
      return false;
    }
  } else {
    // El archivo ya existe
    std::cout << "File '" << rolesFileName
              << "' already exists. Skipping creation." << std::endl;
    return true;
  }
}

bool RightsValidation::initialize() {
  std::cout << "\n=== Initializing Rights Validation System ===" << std::endl;

  // Crear archivo roles.txt en el directorio actual (root)
  if (!createRolesFile()) {
    std::cerr << "Error: Failed to create roles file." << std::endl;
    return false;
  }

  std::cout << "Rights validation system initialized successfully."
            << std::endl;
  std::cout << "File: " << rolesFileName << " (in root directory)" << std::endl;
  std::cout << "==============================================\n" << std::endl;

  return true;
}

bool RightsValidation::roleExists(int roleId) {
    // placeholder implementation
    return false; 
}

// Método para agregar un nuevo rol
bool RightsValidation::addRole(int id, const std::string &roleName) {
  // Crear la nueva línea en el formato correcto
  std::string newRoleLine = std::to_string(id) + ";" + roleName + ";";

  // TODO: Leer el contenido actual del archivo

  std::cout << "Adding role: " << id << " - " << roleName << std::endl;

  // TODO:
  // 1. Leer el contenido actual del archivo
  // 2. Agregar la nueva línea
  // 3. Escribir el contenido actualizado

  // Así se vería la línea que se agregaría
  std::cout << "Would add: " << newRoleLine << std::endl;

  return true;
}

// Método para agregar permisos (vacío por ahora)
bool RightsValidation::addPermissions(int roleId,
    const std::string &permissions) {
  std::cout << "Adding permissions to role " << roleId << ": " << permissions
            << std::endl;
  std::cout << "Permissions functionality not yet implemented." << std::endl;
  return true;
}

std::string RightsValidation::getPermissions(int roleId) {
  // placeholder implementation
  return ""; 
}

bool RightsValidation::removeRole(int roleId) {
  std::cout << "Removing role with ID: " << roleId << std::endl;
  std::cout << "Remove role functionality not yet implemented." << std::endl;
  return true;
}

bool RightsValidation::removePermissions(int roleId,
    const std::string &permissions) {
  std::cout << "Removing permissions from role " << roleId << ": "
            << permissions << std::endl;
  std::cout << "Remove permissions functionality not yet implemented."
            << std::endl;
  return true;
}
