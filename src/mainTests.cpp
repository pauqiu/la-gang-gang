#include "filesystem.h"
#include <iostream>

int main() {
    FileSystem fs("disk.bin");

    std::cout << "=== Testing File Creation ===\n";

    // First, let's see what's in the root directory
    std::cout << "=== Listing root directory ===\n";
    // You might want to add a listDirectory() method

    // Try to create a file
    if (fs.createFile("test.txt")) {
        std::cout << "File created successfully\n";

        // List directory again to see if file appears
        std::cout << "=== Directory after creation ===\n";

        // Try to write to it
        std::string content = "Hello, this is file content!";
        fs.writeFile("test.txt", content);

        // Try to read it
        std::cout << "File content: ";
        fs.readFile("test.txt");
        std::cout << std::endl;

        // Clean up
        /*fs.deleteFile("test.txt");*/

    } else {
        std::cout << "Failed to create file\n";
    }
    return 0;
}