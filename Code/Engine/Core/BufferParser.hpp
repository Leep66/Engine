#pragma once
#include "Engine/Core/BufferUtils.hpp"
#include <string>

struct Vec2;
struct IntVec2;
struct Rgba8;
struct VertexPCU;


class BufferParser
{
public:
	BufferParser(const void* data, size_t size, EndianMode mode = EndianMode::NATIVE);

	void SetEndianMode(EndianMode mode);

	byte_t   ParseByte();
	char     ParseChar();
	uint16_t ParseUShort();
	int16_t  ParseShort();
	uint32_t ParseUInt();
	int32_t  ParseInt();
	uint64_t ParseUInt64();
	int64_t  ParseInt64();
	float    ParseFloat();
	double   ParseDouble();
	bool	 ParseBool();


	std::string ParseStringZeroTerminated();
	std::string ParseStringLengthPreceded();

	Vec2    ParseVec2();
	IntVec2 ParseIntVec2();
	Rgba8   ParseRgba8();
	Rgba8 ParseRgb();

	Vertex_PCU ParseVertexPCU();

	void JumpToOffset(size_t absoluteOffset);
	size_t GetOffset() const;
	size_t GetSize() const;
	EndianMode GetEndianMode() const { return m_mode; }



private:
	void ReadBytes(void* outData, size_t size);

private:
	const byte_t* m_data = nullptr;
	size_t m_size = 0;
	size_t m_offset = 0;
	EndianMode m_mode;
};
