#include "filesystem.h"
#include "test.h"
#include <iostream>

int main(int argc, char* argv[]) {
    // Default behavior: assume disk is not created
    bool diskExists = false;
    
    // Check command line arguments
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--exists" || arg == "-e" || arg == "exists") {
            diskExists = true;
        } else if (arg == "--new" || arg == "-n" || arg == "new") {
            diskExists = false;
        } else {
            std::cout << "Usage: " << argv[0] << " [--exists|--new]\n";
            std::cout << "  --exists, -e, exists: Assume disk already exists and just read files\n";
            std::cout << "  --new, -n, new: Create new disk and run complete tests (default)\n";
            return 1;
        }
    }
    FileSystem fs("disk.bin");
    
    if (diskExists) {
        std::cout << "Mode: DISK ALREADY EXISTS\n";
        std::cout << "Only reading existing files.\n\n";
        readExistingFiles(fs);
    } else {
        std::cout << "Mode: NEW DISK\n";
        std::cout << "Running complete tests and creating files.\n\n";
        runCompleteTests(fs);
    }
    
    return 0;
}