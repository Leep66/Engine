#include "BufferParser.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <stdexcept>

BufferParser::BufferParser(const void* data, size_t size, EndianMode mode)
	: m_data((const byte_t*)data), m_size(size)
{
	if (mode == EndianMode::NATIVE)
	{
		m_mode = IsMachineLittleEndian() ? EndianMode::LITTLE : EndianMode::BIG;
	}
	else
	{
		m_mode = mode;
	}
}

void BufferParser::SetEndianMode(EndianMode mode)
{
	if (mode == EndianMode::NATIVE)
	{
		m_mode = IsMachineLittleEndian() ? EndianMode::LITTLE : EndianMode::BIG;
	}
	else
	{
		m_mode = mode;
	}
}

void BufferParser::ReadBytes(void* outData, size_t size)
{
	if (m_offset + size > m_size)
		throw std::runtime_error("BufferParser read out of bounds");

	const byte_t* src = m_data + m_offset;
	byte_t* dst = (byte_t*)outData;

	bool machineLittle = IsMachineLittleEndian();
	bool wantLittle = (m_mode == EndianMode::LITTLE);

	if (machineLittle == wantLittle)
	{
		for (size_t i = 0; i < size; ++i) dst[i] = src[i];
	}
	else
	{
		for (size_t i = 0; i < size; ++i) dst[i] = src[size - 1 - i];
	}

	m_offset += size;
}



byte_t BufferParser::ParseByte()
{
	byte_t v;
	ReadBytes(&v, sizeof(v));
	return v;
}

char BufferParser::ParseChar()
{
	char v;
	ReadBytes(&v, sizeof(v));
	return v;
}

uint16_t BufferParser::ParseUShort()
{
	uint16_t v;
	ReadBytes(&v, sizeof(v));
	return v;
}

int16_t BufferParser::ParseShort()
{
	int16_t v;
	ReadBytes(&v, sizeof(v));
	return v;
}

uint32_t BufferParser::ParseUInt()
{
	uint32_t v;
	ReadBytes(&v, sizeof(v));
	return v;
}

int32_t BufferParser::ParseInt()
{
	int32_t v;
	ReadBytes(&v, sizeof(v));
	return v;
}

uint64_t BufferParser::ParseUInt64()
{
	uint64_t v;
	ReadBytes(&v, sizeof(v));
	return v;
}

int64_t BufferParser::ParseInt64()
{
	int64_t v;
	ReadBytes(&v, sizeof(v));
	return v;
}

float BufferParser::ParseFloat()
{
	float v;
	ReadBytes(&v, sizeof(v));
	return v;
}

double BufferParser::ParseDouble()
{
	double v;
	ReadBytes(&v, sizeof(v));
	return v;
}

bool BufferParser::ParseBool()
{
	byte_t b = ParseByte();
	return b != 0;
}

std::string BufferParser::ParseStringZeroTerminated()
{
	std::string result;

	while (true)
	{
		if (m_offset >= m_size)
		{
			throw std::runtime_error("String parse out of bounds");
		}

		char c = (char)m_data[m_offset++];
		if (c == '\0')
			break;

		result.push_back(c);
	}

	return result;
}

std::string BufferParser::ParseStringLengthPreceded()
{
	uint32_t length = ParseUInt();

	if (m_offset + length > m_size)
	{
		throw std::runtime_error("String length exceeds buffer");
	}

	std::string result((const char*)(m_data + m_offset), length);
	m_offset += length;

	return result;
}

Vec2 BufferParser::ParseVec2()
{
	Vec2 v;
	v.x = ParseFloat();
	v.y = ParseFloat();
	return v;
}

IntVec2 BufferParser::ParseIntVec2()
{
	IntVec2 v;
	v.x = ParseInt();
	v.y = ParseInt();
	return v;
}

Rgba8 BufferParser::ParseRgba8()
{
	Rgba8 c;
	c.r = (unsigned char)ParseByte();
	c.g = (unsigned char)ParseByte();
	c.b = (unsigned char)ParseByte();
	c.a = (unsigned char)ParseByte();
	return c;
}

Rgba8 BufferParser::ParseRgb()
{
	Rgba8 c;
	c.r = (unsigned char)ParseByte();
	c.g = (unsigned char)ParseByte();
	c.b = (unsigned char)ParseByte();
	c.a = 255; 
	return c;
}
Vertex_PCU BufferParser::ParseVertexPCU()
{
	Vertex_PCU v;

	v.m_position.x = ParseFloat();
	v.m_position.y = ParseFloat();
	v.m_position.z = ParseFloat();

	v.m_color = ParseRgba8();

	v.m_uvTexCoords.x = ParseFloat();
	v.m_uvTexCoords.y = ParseFloat();

	return v;
}


void BufferParser::JumpToOffset(size_t absoluteOffset)
{
	if (absoluteOffset > m_size)
	{
		throw std::runtime_error("JumpToOffset out of range");
	}
	m_offset = absoluteOffset;
}

size_t BufferParser::GetOffset() const
{
	return m_offset;
}

size_t BufferParser::GetSize() const
{
	return m_size;
}
