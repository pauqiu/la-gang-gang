#include "../../include/RightsValidation/RoleManager.h"
#include "../../include/RightsValidation/RolesFileManager.h"
#include <algorithm>
#include <iostream>
#include <sstream>

RoleManager::RoleManager(FileSystem *filesystem) : fs(filesystem) {}

std::vector<std::string> RoleManager::readRolesFile() {
  RolesFileManager fileManager(fs);
  return fileManager.readRolesFile();
}

bool RoleManager::writeRolesFile(const std::vector<std::string> &lines) {
  RolesFileManager fileManager(fs);
  return fileManager.writeRolesFile(lines);
}

std::string RoleManager::buildRoleLine(const std::string &idStr,
                                       const std::string &roleName,
                                       const std::string &permissions) {
  return idStr + ";" + roleName + ";" + permissions;
}

bool RoleManager::roleExists(int roleId) {
  std::vector<std::string> lines = readRolesFile();

  for (const auto &line : lines) {
    if (line.empty() || line[0] == '#') {
      continue;
    }

    std::istringstream iss(line);
    std::string idStr;
    if (std::getline(iss, idStr, ';')) {
      try {
        int currentId = std::stoi(idStr);
        if (currentId == roleId) {
          return true;
        }
      } catch (const std::exception &e) {
        continue;
      }
    }
  }
  return false;
}

bool RoleManager::addRole(int id, const std::string &roleName) {
  if (id < 0) {
    std::cerr << "Error: Role ID cannot be negative." << std::endl;
    return false;
  }

  if (roleName.empty()) {
    std::cerr << "Error: Role name cannot be empty." << std::endl;
    return false;
  }

  if (roleExists(id)) {
    std::cerr << "Error: Role with ID " << id << " already exists."
              << std::endl;
    return false;
  }

  std::vector<std::string> lines = readRolesFile();
  std::string newRoleLine = buildRoleLine(std::to_string(id), roleName, "");

  bool added = false;
  std::vector<std::string> newLines;

  for (const auto &line : lines) {
    newLines.push_back(line);
    if (!added && (line.empty() || line.find("#") == std::string::npos)) {
      newLines.push_back(newRoleLine);
      added = true;
    }
  }

  if (!added) {
    newLines.push_back(newRoleLine);
  }

  if (writeRolesFile(newLines)) {
    std::cout << "Successfully added role: " << id << " - " << roleName
              << std::endl;
    return true;
  } else {
    std::cerr << "Error: Failed to write role to file." << std::endl;
    return false;
  }
}

bool RoleManager::removeRole(int roleId) {
  // Verificar que el rol existe
  if (!roleExists(roleId)) {
    std::cerr << "Error: Role with ID " << roleId << " does not exist."
              << std::endl;
    return false;
  }

  // Leer todas las líneas del archivo
  std::vector<std::string> lines = readRolesFile();
  std::vector<std::string> newLines;
  bool removed = false;

  for (const auto &line : lines) {
    // Mantener líneas de comentario y líneas vacías
    if (line.empty() || line[0] == '#') {
      newLines.push_back(line);
      continue;
    }

    // Parsear la línea para verificar el ID
    std::istringstream iss(line);
    std::string idStr;
    if (std::getline(iss, idStr, ';')) {
      try {
        int currentId = std::stoi(idStr);
        if (currentId == roleId) {
          // Saltar esta línea (eliminar el rol)
          removed = true;
          std::cout << "Removed role: " << roleId << " - " << line << std::endl;
          continue; // No agregar esta línea a newLines
        }
      } catch (const std::exception &e) {
        // Si no se puede parsear, mantener la línea
        newLines.push_back(line);
        continue;
      }
    }

    // Si no es el rol a eliminar, mantener la línea
    newLines.push_back(line);
  }

  if (!removed) {
    std::cerr << "Error: Could not find role with ID " << roleId
              << " to remove." << std::endl;
    return false;
  }

  // Escribir el archivo actualizado
  if (writeRolesFile(newLines)) {
    std::cout << "Successfully removed role with ID: " << roleId << std::endl;
    return true;
  } else {
    std::cerr << "Error: Failed to write updated roles file." << std::endl;
    return false;
  }
}

bool RoleManager::updateRole(const std::string &oldRoleName,
                             const std::string &newRoleName,
                             const std::string &newPermissions) {
    std::vector<std::string> lines = readRolesFile();
    bool found = false;

    for (auto &line : lines) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string idStr, roleName, permissions;

        if (std::getline(iss, idStr, ';') &&
            std::getline(iss, roleName, ';') &&
            std::getline(iss, permissions)) {

            if (roleName == oldRoleName) {
                line = buildRoleLine(idStr, newRoleName, newPermissions);
                found = true;
                break;
            }
        }
    }

    if (!found) {
        std::cerr << "Error: Role '" << oldRoleName << "' not found." << std::endl;
        return false;
    }

    if (writeRolesFile(lines)) {
        std::cout << "Role updated: " << oldRoleName
                  << " -> " << newRoleName
                  << " | New permissions: " << newPermissions
                  << std::endl;
        return true;
    } else {
        std::cerr << "Error: Failed to save updated roles file." << std::endl;
        return false;
    }
}

int RoleManager::getRoleIdByName(const std::string& roleName) {
    std::vector<std::string> lines = readRolesFile();

    for (const auto &line : lines) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string idStr, currentRoleName;

        if (std::getline(iss, idStr, ';') &&
            std::getline(iss, currentRoleName, ';')) {
            try {
                if (currentRoleName == roleName) {
                    return std::stoi(idStr);
                }
            } catch (const std::exception &e) {
                continue;
            }
        }
    }

    return -1;
}
