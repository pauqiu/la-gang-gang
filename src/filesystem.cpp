#include "filesystem.h"

#include <iostream>

FileSystem::FileSystem(std::string diskName) : diskName(diskName)
{
  // Check if the disk file already exists.
  this->diskFile.open(diskName, std::ios::in | std::ios::out | std::ios::binary);
  
  if (!this->diskFile.is_open()) {
    std::cout << "Disk does not exist. Creating a new disk..." << std::endl;
    initializeDisk();
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
  this->superBlock = {};

  this->superBlock.magicNumber = 0xACBD0005;
  this->superBlock.totalBlocks = MAX_BLOCKS;
  this->superBlock.rootInode = 0;

  // initialize block bitmap
  this->blockBitmap = std::vector<bool>(MAX_BLOCKS, false);
  
  // initialize inode bitmap
  this->inodeBitmap = std::vector<bool>(TOTAL_INODES, false);

  saveMetaData();
}

void FileSystem::saveMetaData()
{
  this->diskFile.open(this->diskName, std::ios::in | std::ios::out | std::ios::binary);

  // write superblock
  this->diskFile.seekp(0);
  this->diskFile.write(reinterpret_cast<const char*>(&this->superBlock), sizeof(SuperBlock));
  
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