#ifndef ROLESFILEMANAGER_H
#define ROLESFILEMANAGER_H

#include "filesystem.h"
#include <string>
#include <vector>

class RolesFileManager {
public:
  RolesFileManager(FileSystem *filesystem);
  bool createRolesFile();
  std::vector<std::string> readRolesFile();
  bool writeRolesFile(const std::vector<std::string> &lines);

private:
  FileSystem *fs;
  static const std::string rolesFileName;
};

#endif