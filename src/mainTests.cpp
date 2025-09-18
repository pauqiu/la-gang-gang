#include "filesystem.h"
#include <iostream>

int main() {
    // Step 1: Initialize the FileSystem with a new disk name
    FileSystem fs("disk.bin");

    std::cout << "\n--- Creating File ---\n";
    if (fs.createFile("test.txt")) {
        std::cout << "File 'test.txt' created successfully.\n";
    } else {
        std::cout << "Failed to create 'test.txt'.\n";
    }

    /*std::cout << "\n--- Creating Directory ---\n";
    if (fs.createDirectory("myDir")) {
        std::cout << "Directory 'myDir' created successfully.\n";
    } else {
        std::cout << "Failed to create 'myDir'.\n";
    }*/

    std::cout << "\n--- Attempting to Read 'nonexistent.txt' ---\n";
    fs.readFile("nonexistent.txt");

    std::cout << "\n--- Deleting 'test.txt' ---\n";
    fs.deleteFile("test.txt");

    std::cout << "\n--- Done ---\n";

    return 0;
}