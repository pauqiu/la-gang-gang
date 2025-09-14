#include "filesystem.h"

FileSystem::FileSystem(std::string diskName) 
{
  this->diskName = diskName;

  // Check if the disk file already exists.
  this->diskFile.open(diskName, std::ios::in | std::ios::out | std::ios::binary);
  
  if (!this->diskFile) {
    initializeDisk();
  } 
   
}

FileSystem::~FileSystem() 
{
  
}

void FileSystem::initializeDisk()
{
  // Create a new file stream object for the disk file.
  std::ofstream newDiskFile(this->diskName, std::ios::in | std::ios::binary);

  // TODO: Initialize the disk file.
}

bool FileSystem::createFile(const std::string fileName)
{
  
}

bool FileSystem::deleteFile(const std::string fileName)
{
  
}

bool FileSystem::readFile(const std::string fileName)
{
  
}

bool FileSystem::writeFile(const std::string fileName, const std::string content)
{
  
}