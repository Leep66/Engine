#pragma once
#include <vector>
#include <string>
#include <cstdint>


int FileReadToBuffer(std::vector<uint8_t>& outBuffer, const std::string& filename);
int FileReadToString(std::string& outString, const std::string& filename);

int FileWriteFromBuffer(std::vector<uint8_t>& inBuffer, const std::string& filename);

bool FileExist(const std::string& filename);
bool FolderExists(const std::string& folderName);
bool CreateFolder(const std::string& folderName);