#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <fstream>
#include <string>

// --- MACROS ---

#define BLOCK_SIZE 256
#define MAX_BLOCKS 8192
#define DISK_SIZE (BLOCK_SIZE * MAX_BLOCKS)
#define MAX_INODES 1638
#define MAX_DIR_ENTRIES 16
#define DIRECT_POINTERS 12
#define MAX_FILE_NAME 12

// --- STRUCTS ---

// Volume Control Block
struct SuperBlock
{
  int magicNumber;  // validates the disk
  int totalBlocks;
  int totalInodes;
  int blockSize;
  int rootInode;
  int freeBlocksCount;
  int freeBlocks[MAX_BLOCKS];
  int freeInodesCount;
  int freeInodes[MAX_INODES];
};

struct dirEntry
{
  char name[MAX_FILE_NAME];
  int inode;
};

// Special kind of file that maps names to inodes
struct Directory
{
  int entriesCount;
  dirEntry entries[MAX_DIR_ENTRIES];
};

// Data of the file
struct Block
{
  char data[BLOCK_SIZE];
  
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
    vector<bool> inodeBitmap;
    vector<bool> blockBitmap;

  // Methods
  private:
    void initializeDisk();
    void setMetaData();
    void loadMetaData();
    void saveMetaData();

  public:
    FileSystem(std::string diskName);
    ~FileSystem();
    bool createFile(const std::string fileName);
    bool deleteFile(const std::string fileName);
    bool readFile(const std::string fileName);
    bool writeFile(const std::string fileName, const std::string content);
    

};

#endif // FILESYSTEM_H