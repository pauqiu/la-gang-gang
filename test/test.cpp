#include <iostream>
#include <test.h>


void runCompleteTests(FileSystem& fs) {
    std::cout << "Running Tests\n\n";
    
    // Run all tests
    testBasicFileOperations(fs);
    testEditFileMethods(fs);
    createAdditionalTestFiles(fs);
    
    std::cout << "\nTests finished\n";
}

void readExistingFiles(FileSystem& fs) {
    std::cout << "Reading existing files from disk \n";

    std::cout << "\nReading doc1.txt \n";
    std::cout << "Content: ";
    fs.readFile("doc1.txt");
    std::cout << std::endl;
    
    std::cout << "\nReading notes.txt \n";
    std::cout << "Content: ";
    fs.readFile("notes.txt");
    std::cout << std::endl;
}

void testBasicFileOperations(FileSystem& fs) {
    std::cout << "Testing File Creation \n";

    // Try to create a file
    if (fs.createFile("test.txt")) {
        std::cout << "File created successfully\n";

        // Try to write to it
        std::string content = "Hello, this is file content!";
        fs.writeFile("test.txt", content);

        // Try to read it
        std::cout << "File content: ";
        fs.readFile("test.txt");
        std::cout << std::endl;
    } else {
        std::cout << "Failed to create file\n";
        return; // Exit if basic creation fails
    }
}

void testEditFileMethods(FileSystem& fs) {
    std::cout << "\nTesting editFile Methods \n";
    
    // replace
    std::cout << "Test 1: Replace 'Hello' with 'Hi' at position 0\n";
    fs.replaceInFile("test.txt", 0, "Hi");
    std::cout << "After replace: ";
    fs.readFile("test.txt");
    std::cout << std::endl;
    
    // insert
    std::cout << "\nTest 2: Insert ' world ' at position 2 \n";
    fs.insertInFile("test.txt", 2, " world ");
    std::cout << "After insert: ";
    fs.readFile("test.txt");
    std::cout << std::endl;
    
    // append
    std::cout << "\nTest 3: Append ' (THE END) ' to file\n";
    fs.appendToFile("test.txt", " (THE END) ");
    std::cout << "After append: ";
    fs.readFile("test.txt");
    std::cout << std::endl;
    
    // overwrite
    std::cout << "\nTest 4: Overwrite 'New overwritten text' to file\n";
    fs.overwriteFile("test.txt", "New overwritten text");
    std::cout << "After overwrite: ";
    fs.readFile("test.txt");
    std::cout << std::endl;
    
    // delete
    std::cout << "\nTesting file deletion:\n";
    fs.deleteFile("test.txt");
}

void createAdditionalTestFiles(FileSystem& fs) {
    std::cout << "\nCreating additional files \n";
    
    // Create first additional file
    if (fs.createFile("doc1.txt")) {
        std::cout << "doc1 created successfully\n";
        fs.writeFile("doc1.txt", "Probando que los archivos se escriban correctamente y luego se puedan leer.");
        std::cout << "doc1 content: ";
        fs.readFile("doc1.txt");
        std::cout << std::endl;
    } else {
        std::cout << "Failed to create doc1.txt\n";
    }
    
    // Create second additional file
    if (fs.createFile("notes.txt")) {
        std::cout << "Notes file created successfully\n";
        fs.writeFile("notes.txt", "Important notes:\n1. Etapa 2 el martes :(\n2. Mil examenes la otra semana\n3. Hacer cafecito ");
        std::cout << "Notes content: ";
        fs.readFile("notes.txt");
        std::cout << std::endl;
    } else {
        std::cout << "Failed to create notes.txt\n";
    }
}