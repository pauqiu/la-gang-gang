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
    // Initialize root directory
    currentDirectory = 0; // Root directory is inode 0
  } else {
    loadMetaData();
    currentDirectory = superBlock.rootInode; // Set to root directory
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

    this->diskFile.open(this->diskName, std::ios::in | std::ios::out | std::ios::binary);
    
    // Create root inode
    Inode rootInode;
    rootInode.id = 0;
    rootInode.fileType = 1; // Directory
    rootInode.fileSize = 0;
    rootInode.isFree = false;
    rootInode.directPointers = std::vector<int>(DIRECT_POINTERS, -1);
    rootInode.indirectPointer = -1;
    rootInode.doubleIndirectPointer = -1;

    // Allocate first data block for root directory
    int rootBlockIndex = allocateBlock();
    if (rootBlockIndex != -1) {
      rootInode.directPointers[0] = rootBlockIndex;
      
      // Initialize the directory block with empty entries
      DataBlock rootBlock;
      memset(&rootBlock, 0, BLOCK_SIZE);

      dirEntry* entries = reinterpret_cast<dirEntry*>(rootBlock.data);
      for (int i = 0; i < BLOCK_SIZE / sizeof(dirEntry); i++) {
        entries[i].inode = -1;
        memset(entries[i].name, 0, MAX_FILE_NAME);
      }
      
      // Add self-reference (.)
      dirEntry selfEntry;
      memset(&selfEntry, 0, sizeof(dirEntry)); // Limpiar primero
      strncpy(selfEntry.name, ".", MAX_FILE_NAME - 1);
      selfEntry.name[MAX_FILE_NAME - 1] = '\0';
      selfEntry.inode = 0;
      entries[0] = selfEntry;

      writeBlock(rootBlockIndex, &rootBlock);
    }

    // Mark inode 0 as used
    this->inodeBitmap[0] = true;
    writeInode(0, rootInode);

    saveMetaData();
  }

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
  
  std::cout << "DEBUG: Loaded superblock, magic: " << std::hex << this->superBlock.magicNumber 
            << ", rootInode: " << std::dec << this->superBlock.rootInode << std::endl;

  unpackInodeBitmap();

  // Debug: check if inode 0 is marked as used
  std::cout << "DEBUG: Inode 0 used: " << this->inodeBitmap[0] << std::endl;
  
  loadBlockBitmap();
}

void FileSystem::saveMetaData()
{
  this->diskFile.seekp(0);

  // write superblock
  this->diskFile.write(reinterpret_cast<const char*>(&this->superBlock), sizeof(SuperBlock));

  // write inode bitmap
  saveInodeBitmap();

  this->diskFile.flush();
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
      inode.indirectPointer = -1;
      inode.doubleIndirectPointer = -1;
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

void FileSystem::readFromDirectPointers(Inode& inode, int& remainingBytes)
{
    DataBlock fileDataBlock;

    for (int i = 0; i < DIRECT_POINTERS && remainingBytes > 0; i++) {
        if (inode.directPointers[i] == -1) continue;

        readBlock(inode.directPointers[i], &fileDataBlock);

        int bytesToPrint = std::min(remainingBytes, BLOCK_SIZE);
        std::cout.write(fileDataBlock.data, bytesToPrint);
        remainingBytes -= bytesToPrint;
    }
}

void FileSystem::readFromIndirectPointer(int indexBlock, int& remainingBytes) {
    if (indexBlock == -1) return;

    std::vector<int> pointers = readIndexBlock(indexBlock);
    DataBlock fileDataBlock;

    for (int pointer : pointers) {
        if (pointer == -1 || remainingBytes <= 0) break;

        readBlock(pointer, &fileDataBlock);
        int bytesToPrint = std::min(remainingBytes, BLOCK_SIZE);
        std::cout.write(fileDataBlock.data, bytesToPrint);
        remainingBytes -= bytesToPrint;
    }
}

void FileSystem::readFromDoubleIndirectPointer(int indexBlock, int& remainingBytes) {
    if (indexBlock == -1) return;

    std::vector<int> indexBlocks = readIndexBlock(indexBlock);

    for (int index : indexBlocks) {
        if (index == -1 || remainingBytes <= 0) break;
        readFromIndirectPointer(index, remainingBytes);
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
  
  // Write basic fields
  this->diskFile.write(reinterpret_cast<const char*>(&inode.id), sizeof(inode.id));
  this->diskFile.write(reinterpret_cast<const char*>(&inode.fileType), sizeof(inode.fileType));
  this->diskFile.write(reinterpret_cast<const char*>(&inode.fileSize), sizeof(inode.fileSize));
  this->diskFile.write(reinterpret_cast<const char*>(&inode.isFree), sizeof(inode.isFree));
  
  // Write direct pointers
  for (int i = 0; i < DIRECT_POINTERS; i++) {
      this->diskFile.write(reinterpret_cast<const char*>(&inode.directPointers[i]), sizeof(int));
  }
  
  // Write indirect pointers
  this->diskFile.write(reinterpret_cast<const char*>(&inode.indirectPointer), sizeof(inode.indirectPointer));
  this->diskFile.write(reinterpret_cast<const char*>(&inode.doubleIndirectPointer), sizeof(inode.doubleIndirectPointer));
  
  this->diskFile.flush();
}

int FileSystem::findFreeBlock() {
  for (int i = 0; i < MAX_DATA_BLOCKS; i++) {
    if (!blockBitmap[i]) {
      return i;
    }
  }
  return -1;
}

int FileSystem::allocateBlock() {
  int block = findFreeBlock();
  if (block != -1) {
    blockBitmap[block] = true;
  }
  return block;
}

void FileSystem::readInode(int inodeIndex, Inode& inode)
{
  long offset = (INODE_TABLE_START + inodeIndex) * BLOCK_SIZE;
  this->diskFile.seekg(offset);
  
  // Read basic fields
  this->diskFile.read(reinterpret_cast<char*>(&inode.id), sizeof(inode.id));
  this->diskFile.read(reinterpret_cast<char*>(&inode.fileType), sizeof(inode.fileType));
  this->diskFile.read(reinterpret_cast<char*>(&inode.fileSize), sizeof(inode.fileSize));
  this->diskFile.read(reinterpret_cast<char*>(&inode.isFree), sizeof(inode.isFree));
  
  // Read direct pointers
  inode.directPointers.resize(DIRECT_POINTERS);
  for (int i = 0; i < DIRECT_POINTERS; i++) {
      this->diskFile.read(reinterpret_cast<char*>(&inode.directPointers[i]), sizeof(int));
  }
  
  // Read indirect pointers
  this->diskFile.read(reinterpret_cast<char*>(&inode.indirectPointer), sizeof(inode.indirectPointer));
  this->diskFile.read(reinterpret_cast<char*>(&inode.doubleIndirectPointer), sizeof(inode.doubleIndirectPointer));
}

void FileSystem::readBlock(int blockIndex, void* content) 
{
  long offset = (DATA_BLOCK_START + blockIndex) * BLOCK_SIZE;
  this->diskFile.seekg(offset);
  this->diskFile.read(reinterpret_cast<char*>(content), BLOCK_SIZE);
}

void FileSystem::writeBlock(int blockIndex, void* content)
{
  long offset = (DATA_BLOCK_START + blockIndex) * BLOCK_SIZE;
  this->diskFile.seekp(offset);
  this->diskFile.write(reinterpret_cast<const char*>(content), BLOCK_SIZE);
  this->diskFile.flush();
}

void FileSystem::deallocateBlock(int blockIndex) {
  if (blockIndex >= 0 && blockIndex < MAX_DATA_BLOCKS) {
    blockBitmap[blockIndex] = false;
  }
}

void FileSystem::deallocateInode(int inodeIndex) {
  if (inodeIndex >= 0 && inodeIndex < TOTAL_INODES) {
    inodeBitmap[inodeIndex] = false;
  }
}

void FileSystem::deallocateIndirectPointer(int indexBlock)
{
    if (indexBlock < 0) return;

    std::vector<int> pointers = readIndexBlock(indexBlock);

    for (int pointer : pointers) {
        if (pointer != -1) {
            deallocateBlock(pointer);
        }
    }

    deallocateBlock(indexBlock);
}

void FileSystem::deallocateDoubleIndirectPointer(int indexBlock)
{
    if (indexBlock < 0) return;

    std::vector<int> pointers = readIndexBlock(indexBlock);

    for (int pointer : pointers) {
        if (pointer != -1) {
            deallocateIndirectPointer(pointer);
        }
    }

    deallocateBlock(indexBlock);
}

void FileSystem::freeInodeBlocks(Inode& inode)
{
    // Direct pointers
    for (int i = 0; i < DIRECT_POINTERS; i++) {
        if (inode.directPointers[i] != -1) {
            deallocateBlock(inode.directPointers[i]);
            inode.directPointers[i] = -1;
        }
    }

    deallocateIndirectPointer(inode.indirectPointer);
    inode.indirectPointer = -1;
    deallocateDoubleIndirectPointer(inode.doubleIndirectPointer);
    inode.doubleIndirectPointer = -1;
}

void FileSystem::markInodeAsFree(int inodeIndex, Inode& inode)
{
    inode.isFree = true;
    inode.fileSize = 0;
    writeInode(inodeIndex, inode);
    deallocateInode(inodeIndex);
}

bool FileSystem::createFile(const std::string fileName)
{
  if (fileName.length() >= MAX_FILE_NAME) {
    std::cerr << "Error: Filename too long. Maximum is " << (MAX_FILE_NAME - 1) << " characters." << std::endl;
    return false;
  }
  std::cout << "DEBUG: Creating file: '" << fileName << "' (length: " << fileName.length() << ")" << std::endl;
  int newInode = allocateInode(0);
  if (newInode == -1)
  {
    std::cerr << "No hay espacio para crear el archivo." << std::endl;
    return false;  
  }

  // Add file to current directory
  if (addToDirectory(currentDirectory, fileName, newInode)) {
    std::cout << "Archivo creado (inodo" << newInode << ")" << std::endl;
    return true;
  } else {
    deallocateInode(newInode);
    std::cerr << "No se pudo agregar el archivo al directorio." << std::endl;
    return false;
  }
}

void FileSystem::deleteFile(const std::string fileName)
{
  int inodeIndex = findInDirectory(this->currentDirectory, fileName);
  if (inodeIndex == -1) {
      std::cout << "Archivo no encontrado\n";
      return;
  }

  Inode inode;
  readInode(inodeIndex, inode);
  freeInodeBlocks(inode);
  markInodeAsFree(inodeIndex, inode);
  removeFromDirectory(this->currentDirectory, inodeIndex);
  std::cout << "Archivo '" << fileName << "' eliminado correctamente.\n";
}

void FileSystem::readFile(const std::string fileName)
{
  int inodeIndex = findInDirectory(this->currentDirectory, fileName);
  if (inodeIndex == -1) {
      std::cout << "Archivo no encontrado\n";
      return;
  }

  Inode inode;
  readInode(inodeIndex, inode);
  int remainingBytes = inode.fileSize;

  readFromDirectPointers(inode, remainingBytes);
  
  if (remainingBytes > 0) {
      readFromIndirectPointer(inode.indirectPointer, remainingBytes);
  }

  if (remainingBytes > 0) {
      readFromDoubleIndirectPointer(inode.doubleIndirectPointer, remainingBytes);
  }

}

void FileSystem::writeFile(const std::string fileName, const std::string content) {
  std::cout << "DEBUG: writeFile searching for: '" << fileName << "'" << std::endl;
  // Search for the file in the current directory
  int fileInodeIndex = findInDirectory(currentDirectory, fileName);
  if (fileInodeIndex == -1) {
    std::cerr << "DEBUG: Archivo no encontrado: " << fileName << std::endl;
    std::cerr << "DEBUG: Current directory inode: " << currentDirectory << std::endl;
    return;
  }

  Inode fileInode;
  readInode(fileInodeIndex, fileInode);

  if (fileInode.fileType != 0){
    std::cerr << "No es un archivo regular: " << fileName << std::endl;
    return;
  }

  int contentSize = content.size();
  int blocksNeeded = (contentSize + BLOCK_SIZE - 1) / BLOCK_SIZE;

  // Free the existent blocks if the new content is smaller
  if (contentSize < fileInode.fileSize) {
    std::vector<int> allocatedBlocks = getAllocatedBlocks(fileInodeIndex);
    for (int i = blocksNeeded; i < allocatedBlocks.size(); i++) {
      deallocateBlock(allocatedBlocks[i]);
      fileInode.directPointers[i] = -1; // Clear the pointer
    }
  }

  // Write the content to the blocks
  int bytesWritten = 0;
  for (int blockIndex = 0; blockIndex < blocksNeeded; blockIndex++) {
    int currentBlockPointer = -1;

    // Get or allocate block
    if (blockIndex < DIRECT_POINTERS) {
      currentBlockPointer = fileInode.directPointers[blockIndex];
      
      // Always allocate a new block for writing
      if (currentBlockPointer == -1) {
        currentBlockPointer = allocateBlock();
        if (currentBlockPointer == -1) {
          std::cerr << "Error: No blocks available" << std::endl;
          break;
        }
        fileInode.directPointers[blockIndex] = currentBlockPointer;
        // Save the updated inode immediately
        writeInode(fileInodeIndex, fileInode);
      }
    }
    else {
      // TODO: Implement indirect pointers
      std::cerr << "Error: File too large for direct pointers" << std::endl;
      break;
    }

    // Analyze what part of the content to write in this block
    int blockOffset = blockIndex * BLOCK_SIZE;
    int bytesToWrite = std::min(BLOCK_SIZE, contentSize - blockOffset);

    if (bytesToWrite > 0) {
      DataBlock block;
      std::memcpy(block.data, content.data() + blockOffset, bytesToWrite);
      std::cout << "DEBUG: Writing to block " << currentBlockPointer << " for file block " << blockIndex << std::endl;
      writeBlock(currentBlockPointer, &block);
      bytesWritten += bytesToWrite;
    }
  }

  // Update file metadata
  fileInode.fileSize = contentSize;
  writeInode(fileInodeIndex, fileInode);

  std::cout << "Written " << bytesWritten << " bytes to " << fileName << std::endl;
}

std::vector<int> FileSystem::getAllocatedBlocks(int inodeIndex) {
  std::vector<int> blocks;
  Inode inode;
  readInode(inodeIndex, inode);
  
  // Direct Blocks
  for (int i = 0; i < DIRECT_POINTERS; i++) {
    if (inode.directPointers[i] != -1) {
      blocks.push_back(inode.directPointers[i]);
    }
  }

  return blocks;
}

int FileSystem::findBlockForOffset(Inode& inode, int offset) {
  int blockIndex = offset / BLOCK_SIZE;
  
  if (blockIndex < DIRECT_POINTERS) {
    return inode.directPointers[blockIndex];
  }
  // TODO: Implementar para punteros indirectos
  return -1;
}

int FileSystem::allocateBlockForInode(Inode& inode, int blockIndexInFile) {
  int newBlock = allocateBlock();
  if (newBlock == -1) return -1;

  if (blockIndexInFile < DIRECT_POINTERS) {
    inode.directPointers[blockIndexInFile] = newBlock;
    writeInode(inode.id, inode);
  } else {
    // TODO: Implementar para punteros indirectos
    deallocateBlock(newBlock);
    return -1;
  }

  return newBlock;
}

void FileSystem::writeIndirectBlock(int indirectBlock, std::vector<int>& pointers) {
  writeBlock(indirectBlock, pointers.data());
}

void FileSystem::updateFileSize(int inodeIndex, int newSize)
{
  Inode inode;
  readInode(inodeIndex, inode);
  inode.fileSize = newSize;
  writeInode(inodeIndex, inode);
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

  std::cout << "DEBUG: Searching in directory inode " << inode 
            << ", type: " << dirInode.fileType 
            << ", first block: " << dirInode.directPointers[0] << std::endl;

  if (dirInode.fileType != 1) {
    std::cerr << "DEBUG: Inode " << inode << " is not a directory (type: " << dirInode.fileType << ")" << std::endl;
    return -1;  // Not a directory
  }
  if (dirInode.directPointers[0] == -1) {
    std::cout << "DEBUG: Directory inode " << inode << " is empty" << std::endl;
    return -1;  // Directory is empty
  }

  DataBlock block;
  readBlock(dirInode.directPointers[0], &block);

  dirEntry* entries = reinterpret_cast<dirEntry*>(block.data);
  int numEntries = BLOCK_SIZE / sizeof(dirEntry);
  
  std::cout << "DEBUG: Searching through " << numEntries << " entries" << std::endl;
  
  for (int i = 0; i < numEntries; i++) {
    if (entries[i].inode != -1) {
      std::cout << "DEBUG: Entry " << i << ": '" << entries[i].name << "' -> inode " << entries[i].inode << std::endl;
    }
    // Check if the inode actually exists
    if (entries[i].inode >= TOTAL_INODES) {
      std::cerr << "DEBUG: Invalid inode number: " << entries[i].inode << std::endl;
      continue;
    }
    if (strncmp(entries[i].name, name.c_str(), MAX_FILE_NAME) == 0) {
      return entries[i].inode;
    }
  }
  return -1;
}

void FileSystem::removeFromDirectory(int dirInodeIndex, int targetInode)
{
    Inode dirInode;
    readInode(dirInodeIndex, dirInode);
    DataBlock block;
    readBlock(dirInode.directPointers[0], &block);
    dirEntry* entries = reinterpret_cast<dirEntry*>(block.data);
    int numEntries = BLOCK_SIZE / sizeof(dirEntry);

    for (int i = 0; i < numEntries; i++) {
        if (entries[i].inode == targetInode) {
            entries[i].inode = -1;
            entries[i].name[0] = '\0';
            writeBlock(dirInode.directPointers[0], &block);
            break;
        }
    }
}

bool FileSystem::addToDirectory(int dirInodeIndex, const std::string name, int newInode)
{
  Inode dirInode;
  readInode(dirInodeIndex, dirInode);

  std::cout << "DEBUG: Directory inode " << dirInodeIndex 
    << ", type: " << dirInode.fileType 
    << ", isFree: " << dirInode.isFree << std::endl;

  if (dirInode.fileType != 1) {
    // DEBUG
    std::cerr << "Error: Not a directory (inode " << dirInodeIndex << ")" << std::endl;
    return false; // Not a directory
  }
  
  // DEBUG
  std::cout << "Adding to directory inode " << dirInodeIndex << std::endl;
  std::cout << "First direct pointer: " << dirInode.directPointers[0] << std::endl;
  
  // Get the directory data block
  DataBlock block;
  if (dirInode.directPointers[0] == -1) {
    // DEBUG
    std::cout << "Allocating new block for directory" << std::endl;
    // Allocate first block for directory
    int blockIndex = allocateBlock();
    if (blockIndex == -1) {
      std::cerr << "Error: No blocks available for directory" << std::endl;
      return false;
    }
    dirInode.directPointers[0] = blockIndex;
    writeInode(dirInodeIndex, dirInode);
    
    // Initialize the new block with empty entries
    dirEntry* newEntries = reinterpret_cast<dirEntry*>(block.data);
    for (int i = 0; i < BLOCK_SIZE / sizeof(dirEntry); i++) {
      newEntries[i].inode = -1;
      memset(newEntries[i].name, 0, MAX_FILE_NAME);
    }
  } else {
    std::cout << "Reading existing directory block: " << dirInode.directPointers[0] << std::endl;
    readBlock(dirInode.directPointers[0], &block);
  }
  
  // Find empty slot and add entry
  dirEntry* entries = reinterpret_cast<dirEntry*>(block.data);
  int numEntries = BLOCK_SIZE / sizeof(dirEntry);
  
  for (int i = 0; i < numEntries; i++) {
    if (entries[i].inode == -1) {
      std::cout << "Found empty slot at position " << i << std::endl;
      strncpy(entries[i].name, name.c_str(), MAX_FILE_NAME -  1);
      entries[i].name[MAX_FILE_NAME - 1] = '\0';
      entries[i].inode = newInode;
      writeBlock(dirInode.directPointers[0], &block);
      std::cout << "Added entry: " << name << " -> inode " << newInode << std::endl;
      return true;
    }
  }
  
  std::cerr << "Error: Directory full" << std::endl;
  return false; // Directory full
}

bool FileSystem::changeDirectory(const std::string dirName)
{
  int targetInode = findInDirectory(currentDirectory, dirName);
  if (targetInode == -1) {
    std::cout << "Directory not found: " << dirName << std::endl;
    return false;
  }
  
  Inode targetInodeObj;
  readInode(targetInode, targetInodeObj);
  
  if (targetInodeObj.fileType != 1) {
    std::cout << "Not a directory: " << dirName << std::endl;
    return false;
  }
  
  currentDirectory = targetInode;
  return true;
}
