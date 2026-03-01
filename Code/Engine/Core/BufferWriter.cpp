#include "Engine/Core/BufferWriter.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <iostream>
#include <iomanip>

static void CopyBytesReversed(byte_t* dst, const byte_t* src, size_t size)
{
	for (size_t i = 0; i < size; ++i)
	{
		dst[i] = src[size - 1 - i];
	}
}

BufferWriter::BufferWriter(std::vector<byte_t>& buffer, EndianMode mode)
	: m_buffer(buffer)
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

void BufferWriter::SetEndianMode(EndianMode mode)
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

void BufferWriter::AppendBytes(const void* data, size_t size)
{
	const byte_t* src = (const byte_t*)data;

	bool machineLittle = IsMachineLittleEndian();
	bool wantLittle = (m_mode == EndianMode::LITTLE);

	if (machineLittle == wantLittle)
	{
		m_buffer.insert(m_buffer.end(), src, src + size);
	}
	else
	{
		size_t oldSize = m_buffer.size();
		m_buffer.resize(oldSize + size);

		CopyBytesReversed(&m_buffer[oldSize], src, size);
	}
}

void BufferWriter::AppendByte(byte_t value) { AppendBytes(&value, sizeof(value)); }
void BufferWriter::AppendChar(char value) { AppendBytes(&value, sizeof(value)); }
void BufferWriter::AppendUShort(uint16_t v) { AppendBytes(&v, sizeof(v)); }
void BufferWriter::AppendShort(int16_t v) { AppendBytes(&v, sizeof(v)); }
void BufferWriter::AppendUInt(uint32_t v) { AppendBytes(&v, sizeof(v)); }
void BufferWriter::AppendInt(int32_t v) { AppendBytes(&v, sizeof(v)); }
void BufferWriter::AppendUInt64(uint64_t v) { AppendBytes(&v, sizeof(v)); }
void BufferWriter::AppendInt64(int64_t v) { AppendBytes(&v, sizeof(v)); }
void BufferWriter::AppendFloat(float v) { AppendBytes(&v, sizeof(v)); }
void BufferWriter::AppendDouble(double v) { AppendBytes(&v, sizeof(v)); }

void BufferWriter::AppendRgba8(const Rgba8& c)
{
	AppendByte((byte_t)c.r);
	AppendByte((byte_t)c.g);
	AppendByte((byte_t)c.b);
	AppendByte((byte_t)c.a);
}

void BufferWriter::AppendVec2(const Vec2& v)
{
	AppendFloat(v.x);
	AppendFloat(v.y);
}

void BufferWriter::AppendIntVec2(const IntVec2& v)
{
    AppendInt(v.x);
    AppendInt(v.y);
}

void BufferWriter::AppendVertexPCU(const Vertex_PCU& v)
{
	AppendFloat(v.m_position.x);
	AppendFloat(v.m_position.y);
	AppendFloat(v.m_position.z);

	AppendRgba8(v.m_color);

	AppendFloat(v.m_uvTexCoords.x);
	AppendFloat(v.m_uvTexCoords.y);
}


void BufferWriter::OverwriteUInt32At(size_t offset, uint32_t value)
{
	if (offset + sizeof(uint32_t) > m_buffer.size())
	{
		ERROR_AND_DIE("OverwriteUInt32At out of range");
	}

	byte_t temp[4];
	const byte_t* src = (const byte_t*)&value;

	bool machineLittle = IsMachineLittleEndian();
	bool wantLittle = (m_mode == EndianMode::LITTLE);

	if (machineLittle == wantLittle)
	{
		temp[0] = src[0]; temp[1] = src[1]; temp[2] = src[2]; temp[3] = src[3];
	}
	else
	{
		temp[0] = src[3]; temp[1] = src[2]; temp[2] = src[1]; temp[3] = src[0];
	}

	m_buffer[offset + 0] = temp[0];
	m_buffer[offset + 1] = temp[1];
	m_buffer[offset + 2] = temp[2];
	m_buffer[offset + 3] = temp[3];
}

void BufferWriter::AppendStringZeroTerminated(const std::string& s)
{
	if (!s.empty())
	{
		m_buffer.insert(m_buffer.end(), (const byte_t*)s.data(), (const byte_t*)s.data() + s.size());
	}
	byte_t zero = 0;
	AppendByte(zero);
}

void BufferWriter::AppendStringLengthPreceded(const std::string& s)
{
	AppendUInt((uint32_t)s.size());

	if (!s.empty())
	{
		m_buffer.insert(m_buffer.end(), (const byte_t*)s.data(), (const byte_t*)s.data() + s.size());
	}
}


