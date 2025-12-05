#pragma once
#include "filesystem.h"
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>

// Template para soportar FileSystem y RaidController
template<typename StorageType = FileSystem>
class Logger {
public:
    enum LogLevel {
        INFO,
        WARNING,
        ERROR,
        SUCCESS
    };

    Logger(StorageType* fs, const std::string& logFileName)
        : filesystem(fs), logFile(logFileName) {
        if (filesystem) initializeLogFile();
    }

    void log(LogLevel level, const std::string& message) {
        std::string logEntry = formatLogEntry(level, message);
        if (filesystem) writeToFile(logEntry);
    }

    void info(const std::string& message) {
        log(INFO, message);
    }

    void warning(const std::string& message) {
        log(WARNING, message);
    }

    void error(const std::string& message) {
        log(ERROR, message);
    }

    void success(const std::string& message) {
        log(SUCCESS, message);
    }

private:
    StorageType* filesystem;
    std::string logFile;

    void initializeLogFile() {
        // Verificar si el archivo existe, si no, crearlo
        std::vector<char> existing = filesystem->readFile(logFile);
        if (existing.empty()) {
            filesystem->createFile(logFile);
            std::string header = "=== LOG FILE CREATED ===\n";
            header += "Timestamp: " + getCurrentTimestamp() + "\n";
            header += "===========================\n\n";
            filesystem->appendToFile(logFile, header);
        }
    }

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()
                      ) % 1000;

        std::tm tm_now;
        localtime_r(&time_t_now, &tm_now);

        std::ostringstream oss;
        oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
        oss << "." << std::setfill('0') << std::setw(3) << ms.count();

        return oss.str();
    }

    std::string getLevelString(LogLevel level) {
        switch (level) {
        case INFO:    return "[INFO]   ";
        case WARNING: return "[WARNING]";
        case ERROR:   return "[ERROR]  ";
        case SUCCESS: return "[SUCCESS]";
        default:      return "[UNKNOWN]";
        }
    }

    std::string formatLogEntry(LogLevel level, const std::string& message) {
        std::ostringstream oss;
        oss << getCurrentTimestamp() << " "
            << getLevelString(level) << " "
            << message << "\n";
        return oss.str();
    }

    void writeToFile(const std::string& logEntry) {
        try {
            filesystem->appendToFile(logFile, logEntry);
            std::cout << logEntry;
        } catch (const std::exception& e) {
            std::cerr << "Error writing to log file: " << e.what() << std::endl;
        }
    }
};
