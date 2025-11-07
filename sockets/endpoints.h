#pragma once
#include <string>

void loadEndpoints(const std::string& path);

std::string getAuthIp();
int getAuthPort();
std::string getProxyIp();
int getProxyPort();
std::string getStorageIp();
int getStoragePort();
