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
int FileSystem::allocateInode()
{
  for (int i = 0; i < inodeBitmap.size(); i++) {
    if (!inodeBitmap[i]) {
      inodeBitmap[i] = true;
      return i;
    }
  }
  return -1;
}

bool FileSystem::createFile(const std::string fileName)
{
  int freeInode = -1;
  for (int i = 0; i < inodeBitmap.size(); i++) 
  {
    if (!inodeBitmap[i])
    {
      freeInode = i;
      inodeBitmap[i] = true;
      break;
    }
  }
  if (freeInode == -1) return false;  // no free inode

  //  initialize inode
  Inode node;
  node.id = freeInode;
  node.fileSize = 0;
  node.fileType = 1;  // file

  //  ... guardar en el disco

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