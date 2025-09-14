#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <fstream>
#include <string>

const int BLOCK_SIZE = 255;

// Volume Control Block
struct SuperBlock
{
  int totalBlocks;
  int blockSize;
  int freeBlocksCount;
  int freeBlocks[1000];
  int freeInnodesCount;
  int freeInnodes[1000];
};

// Special kind of file that maps names to inodes
struct Directory
{
  int indexBlock[1000]; 
};

// Data of the file
struct Block
{
  char data[BLOCK_SIZE];
  
};

const int DIRECT_POINTERS = 10;

// File Control Block
struct Innode
{
  int id;
  int type; // 0 for file, 1 for directory
  int size;
  std::string permissions;
  int directPointers[DIRECT_POINTERS];
  int indirectPointer;
};

class FileSystem {
  // Attributes
  private:
    std::string diskName;
    std::fstream diskFile;

  // Methods
  private:
    void initializeDisk();
    

  public:
    FileSystem(std::string diskName);
    ~FileSystem();
    bool createFile(const std::string fileName);
    bool deleteFile(const std::string fileName);
    bool readFile(const std::string fileName);
    bool writeFile(const std::string fileName, const std::string content);
    

};

#endif // FILESYSTEM_H