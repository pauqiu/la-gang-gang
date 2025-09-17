#include "filesystem.h"

#include <iostream>

FileSystem::FileSystem(std::string diskName) : diskName(diskName)
{
  // Initialize the superblock and bitmaps.
  this->superBlock = {};
  this->blockBitmap = std::vector<bool>(MAX_DATA_BLOCKS, false);
  this->inodeBitmap = std::vector<bool>(TOTAL_INODES, false);
  
  // Check if the disk file already exists.
  this->diskFile.open(diskName, std::ios::in | std::ios::out | std::ios::binary);
  
  if (!this->diskFile.is_open()) {
    std::cout << "Disk does not exist. Creating a new disk..." << std::endl;
    initializeDisk();
  } else {
    loadMetaData();
    std::cout << "Disk loaded successfully." << std::endl;
  }
   
}

FileSystem::~FileSystem() 
{
  if (this->diskFile.is_open()) {
    this->diskFile.close();
  }
}

void FileSystem::initializeDisk()
{
  // Create a new file stream object for the disk file.
  std::ofstream newDiskFile(this->diskName, std::ios::out | std::ios::binary);

  if (!newDiskFile.is_open()) {
    std::cerr << "Error creating disk file." << std::endl;
  } else {
    
    char buffer[BLOCK_SIZE] = {0};
    for (int i = 0; i < MAX_BLOCKS; i++) {
        newDiskFile.write(buffer, BLOCK_SIZE);
    }

    newDiskFile.close();

    setMetaData();
  }

  this->diskFile.close();
  std::cout << "Disk created successfully." << std::endl;

}

void FileSystem::setMetaData()
{
  this->superBlock.magicNumber = 0xACBD0005;
  this->superBlock.totalBlocks = MAX_BLOCKS;
  this->superBlock.rootInode = 0;

  saveMetaData();
}

void FileSystem::loadMetaData() 
{ 
  this->diskFile.seekg(0);
  this->diskFile.read(reinterpret_cast<char*>(&this->superBlock), sizeof(SuperBlock));

  unpackInodeBitmap();

  loadBlockBitmap();
}

void FileSystem::saveMetaData()
{
  this->diskFile.open(this->diskName, std::ios::in | std::ios::out | std::ios::binary);

  // write superblock
  this->diskFile.seekp(0);
  this->diskFile.write(reinterpret_cast<const char*>(&this->superBlock), sizeof(SuperBlock));

  // write inode bitmap
  saveInodeBitmap();
  
}

void FileSystem::saveInodeBitmap()
{
  std::vector<uint8_t> packedBitmap((inodeBitmap.size() + 7) / 8, 0);

  for (size_t i = 0; i < inodeBitmap.size(); ++i) {
    if (inodeBitmap[i]) {
      packedBitmap[i / 8] |= (1 << (i % 8));
    }
  }

  this->diskFile.write(reinterpret_cast<const char*>(packedBitmap.data()), packedBitmap.size());
}

void FileSystem::unpackInodeBitmap()
{
  std::vector<uint8_t> packedBitmap((inodeBitmap.size() + 7) / 8, 0);
  this->diskFile.read(reinterpret_cast<char*>(packedBitmap.data()), packedBitmap.size());

  for (size_t i = 0; i < this->inodeBitmap.size(); ++i) {
    size_t byteIndex = i / 8;
    size_t bitIndex = i % 8;
    this->inodeBitmap[i] = (packedBitmap[byteIndex] & (1 << bitIndex)) != 0;
  }
}

int FileSystem::allocateInode(int type)
{
  for (int i = 0; i < inodeBitmap.size(); i++) {
    if (!inodeBitmap[i]) {
      inodeBitmap[i] = true;
      Inode inode;
      inode.id = i;
      inode.fileSize = 0;
      inode.fileType = type;
      inode.isFree = false;
      inode.directPointers = std::vector<int>(DIRECT_POINTERS, -1);
      writeInode(i, inode);  // save inode to disk
      return i;
    }
    
  }
  return -1;
}

void FileSystem::loadBlockBitmap() 
{
  Inode node;

  for (int i = 1; i < TOTAL_INODES + 1; i++) {
    // If the inode is not used, skip it
    if (this->inodeBitmap[i] == false) continue;
    std::cout << this->inodeBitmap[1] << std::endl;
    this->diskFile.read(reinterpret_cast<char*>(&node), sizeof(Inode));
    // Handling direct pointers
    setBlockBitmap(node.directPointers, DIRECT_POINTERS);

    // Handling indirect pointers
    loadIndirectPointers(node.indirectPointer);
  }
}

void FileSystem::loadIndirectPointers(int indexBlock)
{
  if (indexBlock != -1) {
    this->blockBitmap[indexBlock] = true;
    // Read index block and set bitmap
    std::vector<int> pointers = readIndexBlock(indexBlock);
    setBlockBitmap(pointers, POINTERS_PER_INDEX_BLOCK);
    
    loadDoubleIndirectPointers(pointers);
  }
}

void FileSystem::loadDoubleIndirectPointers(std::vector<int> indexBlocks) 
{
  for (int i = 0; i < indexBlocks.size(); i++)
    if (indexBlocks[i] != -1) {
      std::vector<int> pointers = readIndexBlock(indexBlocks[i]);
      setBlockBitmap(pointers, POINTERS_PER_INDEX_BLOCK);
    }
}

void FileSystem::setBlockBitmap(std::vector<int> blocks, int size)
{
  for (int i = 0; i < size; i++) {
    if (blocks[i] == -1) continue;
    this->blockBitmap[blocks[i]] = true;
  }
}

std::vector<int> FileSystem::readIndexBlock(int indexBlock)
{
  std::vector<int> pointers = std::vector<int>(POINTERS_PER_INDEX_BLOCK, -1);
  
  this->diskFile.seekg((DATA_BLOCK_START + indexBlock) * BLOCK_SIZE);
  this->diskFile.read(reinterpret_cast<char*>(&pointers), sizeof(pointers));
  return pointers;
}
void FileSystem::writeInode(int inodeIndex, Inode& inode) 
{ 
  long offset = (INODE_TABLE_START + inodeIndex) * BLOCK_SIZE;
  this->diskFile.seekp(offset);
  this->diskFile.write(reinterpret_cast<const char*>(&inode), sizeof(Inode));
  this->diskFile.flush();
}
void FileSystem::readInode(int inodeIndex, Inode& inode)
{
  long offset = (INODE_TABLE_START + inodeIndex) * BLOCK_SIZE;
  this->diskFile.seekg(offset);
  this->diskFile.read(reinterpret_cast<char*>(&inode), sizeof(Inode));
}

void FileSystem::readBlock(int blockIndex, void* content) 
{
  long offset = (DATA_BLOCK_START + blockIndex) * BLOCK_SIZE;
  this->diskFile.seekg(offset);
  this->diskFile.read(reinterpret_cast<char*>(&content), BLOCK_SIZE);
}
void FileSystem::writeBlock(int blockIndex, void* content)
{
  long offset = (DATA_BLOCK_START + blockIndex) * BLOCK_SIZE;
  this->diskFile.seekp(offset);
  this->diskFile.write(reinterpret_cast<const char*>(content), BLOCK_SIZE);
  this->diskFile.flush();
}


bool FileSystem::createFile(const std::string fileName)
{
  int newInode = allocateInode(0);
  if (newInode == -1)
  {
    std::cerr << "No hay espacio para crear el archivo." << std::endl;
    return false;  
  }

  std::cout << "Archivo creado (inodo" << newInode << ")" << std::endl;
  return true;
}

void FileSystem::deleteFile(const std::string fileName)
{
 
}

void FileSystem::readFile(const std::string fileName)
{
  
}

void FileSystem::writeFile(const std::string fileName, const std::string content)
{
  
}

// directory methods

bool FileSystem::createDirectory(const std::string dirName) 
{
  // Allocate a new inode for the directory
  int newInode = allocateInode(1);
  if (newInode == -1) return false;
  
  // Add the new directory to the current directory
  bool success = addToDirectory(this->currentDirectory, dirName, newInode);
  return success;
}
int FileSystem::findInDirectory(int inode, const std::string name)
{
  Inode dirInode;
  readInode(inode, dirInode);
  
  if (dirInode.fileType != 1) return -1;  // Not a directory
  if (dirInode.directPointers[0] == -1) return -1;  // Directory is empty

  DataBlock block;
  readBlock(dirInode.directPointers[0], &block);

  dirEntry* entries = reinterpret_cast<dirEntry*>(block.data);
  int numEntries = BLOCK_SIZE / sizeof(dirEntry);
  for (int i = 0; i < numEntries; i++) {
    if (entries[i].name == name) {
      return entries[i].inode;
    }
  }
  return -1;
}
bool FileSystem::changeDirectory(const std::string dirName)
{

}
bool FileSystem::addToDirectory(int inode, const std::string name, int newInode)
{

}