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

        std::cout << "\nTesting editFile Methods:\n";
        
        // replace in specific position
        std::cout << "Test 1: Replace 'Hello' with 'Hi' at position 0\n";
        fs.replaceInFile("test.txt", 0, "Hi");
        std::cout << "After replace: ";
        fs.readFile("test.txt");
        std::cout << std::endl;
        
        std::cout << "\nTest 2: Insert ' world ' at position 2 \n";
        fs.insertInFile("test.txt", 2, " world ");
        std::cout << "After insert: ";
        fs.readFile("test.txt");
        std::cout << std::endl;

        std::cout << "\nTest 3: Append ' (THE END) ' to file\n";
        fs.appendToFile("test.txt", " (THE END) ");
        std::cout << "After append: ";
        fs.readFile("test.txt");
        std::cout << std::endl;
        
        std::cout << "\nChecking file was deleted correctly: \n";
        // Clean up
        fs.deleteFile("test.txt");

        // Check if file was deleted correctly.
        fs.readFile("test.txt");
    } else {
        std::cout << "Failed to create file\n";
    }
    return 0;
}