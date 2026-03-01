#pragma once
#include <string>
#include "Engine/Core/BufferUtils.hpp"

struct Rgba8;
struct Vec2;
struct IntVec2;
struct Vertex_PCU;

class BufferWriter
{
public:
	BufferWriter(std::vector<byte_t>& buffer, EndianMode mode = EndianMode::NATIVE);

public:
	void SetEndianMode(EndianMode mode);

	void AppendByte(byte_t value);
	void AppendChar(char value);
	void AppendUShort(uint16_t value);
	void AppendShort(int16_t value);
	void AppendUInt(uint32_t value);
	void AppendInt(int32_t value);
	void AppendUInt64(uint64_t value);
	void AppendInt64(int64_t value);
	void AppendFloat(float value);
	void AppendDouble(double value);
	void AppendRgba8(const Rgba8& c);
	void AppendVec2(const Vec2& v);
	void AppendIntVec2(const IntVec2& v);
	void AppendVertexPCU(const Vertex_PCU& v);
	void OverwriteUInt32At(size_t offset, uint32_t value);


	void AppendStringZeroTerminated(const std::string& s);
	void AppendStringLengthPreceded(const std::string& s);

private:
	void AppendBytes(const void* data, size_t size);

private:
	std::vector<byte_t>& m_buffer;
	EndianMode m_mode;
};
