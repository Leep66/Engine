#include "Engine/Core/FileUtils.hpp"
#include <stdio.h>

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