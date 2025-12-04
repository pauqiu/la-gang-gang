#ifndef DATAINJECTION_H
#define DATAINJECTION_H

#include "filesystem.h"
#include <string>

namespace DataInjection {
    bool ensureSampleData(FileSystem &fs, const std::string &diskName);
    void injectSampleData(FileSystem &fs);
}

#endif // DATAINJECTION_H
