#include "../../include/RightsValidation/RolesFileManager.h"
#include <iostream>
#include <sstream>

const std::string RolesFileManager::rolesFileName = "roles.txt";

RolesFileManager::RolesFileManager(FileSystem *filesystem) : fs(filesystem) {}

bool RolesFileManager::createRolesFile() {
  int fileInode = fs->findInDirectory(fs->getCurrentDirectoryInode(), rolesFileName);

  if (fileInode == -1) {
    if (fs->createFile(rolesFileName)) {
      std::cout << "File '" << rolesFileName << "' created successfully."
                << std::endl;

      std::string content = "# Roles System File\n";
      content += "# Format: id;role_name;permissions\n";
      content +=
          "# Permissions are comma-separated (e.g., read,write,delete)\n";

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
    std::cout << "File '" << rolesFileName
              << "' already exists. Skipping creation." << std::endl;
    return true;
  }
}

std::vector<std::string> RolesFileManager::readRolesFile() {
  std::vector<std::string> lines;

  try {
    std::vector<char> fileContent = fs->readFile(rolesFileName);

    if (!fileContent.empty()) {
      std::string content(fileContent.begin(), fileContent.end());
      std::istringstream iss(content);
      std::string line;

      while (std::getline(iss, line)) {
        if (!line.empty()) {
          lines.push_back(line);
        }
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "Error reading roles file: " << e.what() << std::endl;
  }

  return lines;
}

bool RolesFileManager::writeRolesFile(const std::vector<std::string> &lines) {
  try {
    std::string content;
    for (const auto &line : lines) {
      content += line + "\n";
    }

    fs->overwriteFile(rolesFileName, content);
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error writing roles file: " << e.what() << std::endl;
    return false;
  }
}