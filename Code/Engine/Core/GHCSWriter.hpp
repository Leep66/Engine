#pragma once
#include <vector>
#include <cstdint>
#include "Engine/Core/BufferWriter.hpp"

struct GhcsChunkPatch
{
	size_t sizeFieldOffset = 0;
	size_t payloadStartOffset = 0;
};

struct TocEntry
{
	uint8_t  type;
	uint32_t offset;
	uint32_t size;
};

inline void AppendFourCC(BufferWriter& w, char const* fourcc)
{
	w.AppendChar(fourcc[0]);
	w.AppendChar(fourcc[1]);
	w.AppendChar(fourcc[2]);
	w.AppendChar(fourcc[3]);
}

inline GhcsChunkPatch BeginGhcsChunk(BufferWriter& w, std::vector<byte_t>& buffer, uint8_t chunkType, uint8_t chunkVer = 0)
{
	GhcsChunkPatch patch;

	AppendFourCC(w, "GHCK");

	w.AppendByte((byte_t)chunkType);
	w.AppendByte((byte_t)chunkVer);

	patch.sizeFieldOffset = buffer.size();
	w.AppendUInt(0);

	patch.payloadStartOffset = buffer.size();
	return patch;
}

inline void EndGhcsChunk(BufferWriter& w, std::vector<byte_t>& buffer, GhcsChunkPatch const& patch)
{
	size_t payloadEnd = buffer.size();
	uint32_t payloadSize = (uint32_t)(payloadEnd - patch.payloadStartOffset);

	w.OverwriteUInt32At(patch.sizeFieldOffset, payloadSize);

	AppendFourCC(w, "ENDC");
}