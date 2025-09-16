#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <cstring>
#include <fstream>
#include <string>
#include <vector>

// --- MACROS ---

#define BLOCK_SIZE 256
#define MAX_BLOCKS 8192
#define DISK_SIZE (BLOCK_SIZE * MAX_BLOCKS)
#define TOTAL_INODES 512
#define DIRECT_POINTERS 12
#define MAX_FILE_NAME 12
#define INODE_TABLE_START 1
#define DATA_BLOCK_START (INODE_TABLE_START + TOTAL_INODES)

// --- STRUCTS ---

// Volume Control Block
struct SuperBlock
{
  int magicNumber;  // validates the disk
  int totalBlocks;
  int rootInode;
};

struct dirEntry
{
  char name[MAX_FILE_NAME];
  int inode;
};

// Data of the file
struct DataBlock
{
  char data[BLOCK_SIZE];

  DataBlock() {
      memset(data, 0, BLOCK_SIZE);
  }
  
};

// File Control Block
struct Inode
{
  int id;
  int fileType; // 0 for file, 1 for directory
  int fileSize;
  char permissions[4];
  int directPointers[DIRECT_POINTERS];
  int indirectPointer;
  bool isFree;
};

// --- CLASS ---

class FileSystem {
  // Attributes
  private:
    std::string diskName;
    std::fstream diskFile;
    SuperBlock superBlock;
    std::vector<bool> inodeBitmap;
    std::vector<bool> blockBitmap;

  // Methods
  private:
    void initializeDisk();
    void setMetaData();
    void loadMetaData();
    void saveMetaData();
    void saveInodeBitmap();
    void unpackInodeBitmap();
    //int findFreeInode();
    //int findFreeBlock();
    //int allocateBlock();
    int allocateInode();
    //void deallocateBlock(int blockIndex);
    //void writeBlock(int blockIndex, const std::string content);
    //std::string readBlock(int blockIndex);

  public:
    FileSystem(std::string diskName);
    ~FileSystem();
    bool createFile(const std::string fileName);
    void deleteFile(const std::string fileName);
    void readFile(const std::string fileName);
    void writeFile(const std::string fileName, const std::string content);
  
};

#endif // FILESYSTEM_H