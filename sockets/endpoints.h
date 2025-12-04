#pragma once
#include <string>

void loadEndpoints(const std::string& path);

std::string getAuthIp();
int getAuthPort();
std::string getProxyIp();
int getProxyPort();
std::string getStorageIp();
int getStoragePort();

// Storage2 - opcional para redundancia
bool hasStorage2();
std::string getStorage2Ip();
int getStorage2Port();

// Receptor
bool hasReceptor();
std::string getReceptorIp();
int getReceptorPort();
