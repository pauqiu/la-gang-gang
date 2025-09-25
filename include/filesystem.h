#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

// --- MACROS ---
#define SUPERBLOCK_BLOCK 0
#define INODE_BITMAP_BLOCK 1
#define INODE_TABLE_START 2
#define BLOCK_SIZE 256
#define MAX_BLOCKS 8192
#define MAX_DATA_BLOCKS (MAX_BLOCKS - 1 - TOTAL_INODES)
#define DISK_SIZE (BLOCK_SIZE * MAX_BLOCKS)
#define TOTAL_INODES 512
#define DIRECT_POINTERS 12
#define MAX_FILE_NAME 12
#define POINTERS_PER_INDEX_BLOCK (BLOCK_SIZE / sizeof(int))
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

  dirEntry() 
  {
    memset(name, 0, MAX_FILE_NAME);
    inode = -1;
  }
  dirEntry(const char* name, int inode)
  {
    strncpy(this->name, name, MAX_FILE_NAME - 1);
    this->inode = inode;
  }
};

struct Directory 
{
  int inode;
  std::vector<dirEntry> entries;

  Directory(int inode) : inode(inode) {}
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
#pragma pack(push, 1)
struct Inode
{
  int id;
  int fileType; // 0 for file, 1 for directory
  int fileSize;
  char permissions[4];
  int directPointers[DIRECT_POINTERS];
  int indirectPointer;
  int doubleIndirectPointer;
  bool isFree;

  Inode() {
      id = -1;
      fileType = 0;
      fileSize = 0;
      memset(permissions, 0, 4);
      memset(directPointers, -1, sizeof(directPointers));
      indirectPointer = -1;
      doubleIndirectPointer = -1;
      isFree = true;
  }
};

// --- CLASS ---

class FileSystem {
  // Attributes
  private:
    std::string diskName;
    std::fstream diskFile;
    SuperBlock superBlock;
    Directory rootDirectory;
    std::vector<bool> inodeBitmap;
    std::vector<bool> blockBitmap;
    std::vector<Directory> directories;
    int currentDirectoryInode; // Inode of the current directory

  // Methods
  private:
    void initializeDisk();
    void setMetaData();
    void loadMetaData();
    void saveMetaData();
    void saveInodeBitmap();
    void unpackInodeBitmap();
    void loadBlockBitmap();
    void loadIndirectPointers(int indexBlock);
    void loadDoubleIndirectPointers(std::vector<int> indexBlocks);
    void readFromDirectPointers(Inode& inode, int& remainingBytes);
    void readFromIndirectPointer(int indexBlock, int& remainingBytes);
    void readFromDoubleIndirectPointer(int indexBlock, int& remainingBytes);
    void setBlockBitmap(std::vector<int> blocks, int size);
    std::vector<int> readIndexBlock(int indexBlock);
    void writeInode(int inodeIndex, Inode& inode);
    int findFreeBlock();
    int allocateBlock();
    void readInode(int inodeIndex, Inode& inode);
    int allocateInode(int type);
    void readBlock(int blockIndex, void* content);
    void writeBlock(int blockIndex, void* content);
    std::vector<int> getAllocatedBlocks(int inodeIndex);
    int findBlockForOffset(Inode& inode, int offset);
    int allocateBlockForInode(Inode& inode, int blockIndexInFile);
    void writeIndirectBlock(int indirectBlock, std::vector<int>& pointers);
    void updateFileSize(int inodeIndex, int newSize);
    void deallocateBlock(int blockIndex);
    void deallocateInode(int inodeIndex);
    void deallocateIndirectPointer(int indexBlock);
    void deallocateDoubleIndirectPointer(int indexBlock);
    void freeInodeBlocks(Inode& inode);
    void markInodeAsFree(int inodeIndex, Inode& inode);

    // directory methods
    bool createDirectory(const std::string dirName);
    int findInDirectory(int inode, const std::string name);
    bool changeDirectory(const std::string dirName);
    bool addToDirectory(int inode, const std::string name, int newInode);
    void removeFromDirectory(int dirInodeIndex, int targetInode);

  public:
    FileSystem(std::string diskName);
    ~FileSystem();
    bool createFile(const std::string fileName);
    void deleteFile(const std::string fileName);
    void readFile(const std::string fileName);
    void writeFile(const std::string fileName, const std::string content);

};

#endif // FILESYSTEM_H