#include "Engine/Core/FileUtils.hpp"
#include <stdio.h>
#include <sys/stat.h> 
#include <filesystem>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"

int FileReadToBuffer(std::vector<uint8_t>& outBuffer, const std::string& filename)
{
	FILE* file;

	errno_t err = fopen_s(&file, filename.c_str(), "rb");
	if (err != 0 || file == nullptr) {
		return -1;
	}

	fseek(file, 0, SEEK_END); 
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (fileSize <= 0) {
		fclose(file);
		return -1;
	}

	outBuffer.resize(fileSize);

	size_t bytesRead = fread(outBuffer.data(), 1, fileSize, file);
	fclose(file);

	if (bytesRead != static_cast<size_t>(fileSize)) {
		return -1;
	}

	return static_cast<int>(bytesRead);
}

int FileReadToString(std::string& outString, const std::string& filename)
{
	std::vector<uint8_t> buffer;
	int bytesRead = FileReadToBuffer(buffer, filename);
	if (bytesRead <= 0) {
		return -1;
	}

	outString.assign(buffer.begin(), buffer.end());
	return bytesRead;
}

int FileWriteFromBuffer(std::vector<uint8_t>& inBuffer, const std::string& filename)
{
	FILE* fp = nullptr;
	errno_t err = fopen_s(&fp, filename.c_str(), "wb");
	if (err != 0 || fp == nullptr) 
	{
		return -1;
	}

	size_t elemSize = sizeof(uint8_t);
	size_t count = inBuffer.size();
	size_t written = fwrite(inBuffer.data(), elemSize, count, fp);

	fclose(fp);

	if (written != count) 
	{
		return -1;
	}

	return static_cast<int>(written);
}

bool FileExist(const std::string& filename)
{
	return std::filesystem::exists(filename);
}

bool FolderExists(const std::string& folderName)
{
	return std::filesystem::is_directory(folderName);
}

bool CreateFolder(const std::string& folderName)
{
	if (!std::filesystem::create_directory(folderName))
	{
		ERROR_RECOVERABLE(Stringf("Could not create folder: \"%s\"", folderName.c_str()));
		return false;
	}
	return true;
}

