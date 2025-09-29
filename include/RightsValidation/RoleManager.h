#ifndef ROLEMANAGER_H
#define ROLEMANAGER_H

#include "filesystem.h"
#include <string>
#include <vector>

class RoleManager {
public:
  RoleManager(FileSystem *filesystem);
  bool addRole(int id, const std::string &roleName);
  bool removeRole(int roleId);
  bool roleExists(int roleId);

private:
  FileSystem *fs;
  std::string buildRoleLine(const std::string &idStr,
                            const std::string &roleName,
                            const std::string &permissions = "");
  std::vector<std::string> readRolesFile();
  bool writeRolesFile(const std::vector<std::string> &lines);
};

#endif