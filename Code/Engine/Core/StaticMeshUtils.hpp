#pragma once
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/OBB3.hpp"
#include <vector>
#include <string>

bool LoadStaticMeshFile(std::vector<Vertex_PCUTBN>& verts, std::string const& filePathNoExtension);
bool LoadOBJMeshFile(std::vector<Vertex_PCUTBN>& verts, std::string const& objFilePath);
void ComputeMissingNormals(std::vector<Vertex_PCUTBN>& verts);
void ComputeMissingTangentsBitanges(std::vector<Vertex_PCUTBN>& verts);
	
bool ParseOBJMeshTextBuffer(std::vector<Vertex_PCUTBN>& verts, std::string const& objFileContents, ...);
bool ParseOBJMeshTextBuffer_UseSplitStrings(std::vector<Vertex_PCUTBN>& verts, ...);