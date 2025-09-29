#ifndef RIGHTS_VALIDATION_H
#define RIGHTS_VALIDATION_H

#include <string>
#include "filesystem.h"

class RightsValidation {
private:
    FileSystem* fs;
    std::string rolesFileName = "roles.txt";
    
    bool createRolesFile();

public:
    RightsValidation(FileSystem* filesystem);
    bool initialize();
};

#endif