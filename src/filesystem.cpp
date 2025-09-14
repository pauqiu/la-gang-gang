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
  if (this->disk_file.is_open()) {
    this->disk_file.close();
  }
}

void FileSystem::initializeDisk()
{
  // Create a new file stream object for the disk file.
  std::ofstream newDiskFile(this->diskName, std::ios::out | std::ios::binary);

  if (!newDiskFile.is_open()) {
    std::cerr << "Error creating disk file." << std::endl;
  } else {
    // TODO: Initialize the disk file.
  }

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