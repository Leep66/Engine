#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include "Engine/Core/DevConsole.hpp"

extern DevConsole* g_theDevConsole;

using byte_t = unsigned char;

enum class EndianMode
{
	NATIVE,
	LITTLE,
	BIG
};

inline bool IsMachineLittleEndian()
{
	uint16_t v = 0x1;
	return *((byte_t*)&v) == 0x1;
}

inline void PrintBufferHexOnDevConsole(const std::vector<byte_t>& buffer)
{
	std::string hex;
	hex.reserve(buffer.size() * 3);
	for (byte_t b : buffer)
	{
		hex += Stringf("%02X ", (unsigned)b);
	}
	g_theDevConsole->AddLine(Rgba8::WHITE, hex.c_str());

}



