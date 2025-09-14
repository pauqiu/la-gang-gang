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

}

void FileSystem::setMetaData()
{
  this->superBlock = {};

  std::cout << superBlock.totalBlocks << std::endl;

  
  
}

void FileSystem::saveMetaData()
{
  
}

bool FileSystem::createFile(const std::string fileName)
{
  return true;
}

bool FileSystem::deleteFile(const std::string fileName)
{
  return true;
}

bool FileSystem::readFile(const std::string fileName)
{
  return true;
}

bool FileSystem::writeFile(const std::string fileName, const std::string content)
{
  return true;
}