#ifndef RAID_CONTROLLER_H
#define RAID_CONTROLLER_H

#include "filesystem.h"
#include <iostream>
#include <string>

/**
 * RAID 1 Controller - Mirroring
 * 
 * Escribe datos en dos discos simultáneamente.
 * Lee del primario; si falla, lee del espejo.
 * Transparente para el usuario: misma interfaz que FileSystem.
 */
class RaidController {
private:
    FileSystem* primary;    // Disco primario
    FileSystem* mirror;     // Disco espejo

    void logStatus(const std::string& operation, bool primaryOk, bool mirrorOk) {
        if (!primaryOk && !mirrorOk) {
            std::cerr << "[RAID1] ERROR: " << operation << " falló en AMBOS discos\n";
        } else if (!primaryOk) {
            std::cerr << "[RAID1] WARNING: " << operation << " falló en disco primario\n";
        } else if (!mirrorOk) {
            std::cerr << "[RAID1] WARNING: " << operation << " falló en disco espejo\n";
        }
    }

public:
    RaidController(FileSystem* primaryDisk, FileSystem* mirrorDisk)
        : primary(primaryDisk), mirror(mirrorDisk) {
        std::cout << "[RAID1] Controlador inicializado con 2 discos (mirroring)\n";
    }

    // Crear archivo en ambos discos
    bool createFile(const std::string& fileName) {
        bool p = primary->createFile(fileName);
        bool m = mirror->createFile(fileName);
        logStatus("createFile(" + fileName + ")", p, m);
        return p || m;
    }

    // Eliminar archivo de ambos discos
    void deleteFile(const std::string& fileName) {
        primary->deleteFile(fileName);
        mirror->deleteFile(fileName);
    }

    // Leer: intentar primario, si falla usar espejo
    std::vector<char> readFile(const std::string fileName) {
        std::vector<char> data = primary->readFile(fileName);
        if (!data.empty()) {
            return data;
        }
        std::cout << "[RAID1] Primario vacío/falló, leyendo de espejo\n";
        return mirror->readFile(fileName);
    }

    // Escribir en ambos discos
    void writeFile(const std::string fileName, const std::string content) {
        primary->writeFile(fileName, content);
        mirror->writeFile(fileName, content);
    }

    int findInDirectory(int inode, const std::string name) {
        int result = primary->findInDirectory(inode, name);
        if (result != -1) return result;
        return mirror->findInDirectory(inode, name);
    }

    int getCurrentDirectoryInode() {
        return primary->getCurrentDirectoryInode();
    }

    void replaceInFile(const std::string& fileName, int position, const std::string& newContent) {
        primary->replaceInFile(fileName, position, newContent);
        mirror->replaceInFile(fileName, position, newContent);
    }

    void insertInFile(const std::string& fileName, int position, const std::string& content) {
        primary->insertInFile(fileName, position, content);
        mirror->insertInFile(fileName, position, content);
    }

    void appendToFile(const std::string& fileName, const std::string& content) {
        primary->appendToFile(fileName, content);
        mirror->appendToFile(fileName, content);
    }

    void overwriteFile(const std::string& fileName, const std::string& content) {
        primary->overwriteFile(fileName, content);
        mirror->overwriteFile(fileName, content);
    }

    // Acceso a discos individuales (para diagnóstico/logs)
    FileSystem* getPrimaryDisk() { return primary; }
    FileSystem* getMirrorDisk() { return mirror; }
};

#endif // RAID_CONTROLLER_H
