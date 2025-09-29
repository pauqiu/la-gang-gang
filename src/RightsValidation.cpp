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
