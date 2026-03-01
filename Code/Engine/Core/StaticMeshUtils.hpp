#pragma once
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Core/FileUtils.hpp"
#include <vector>
#include <string>

bool LoadStaticMeshFile(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices, std::string const& filePathNoExtension, Mat44 const& transform = Mat44());
bool LoadOBJMeshFile(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices, std::string const& objFilePath, Mat44 const& transform = Mat44());
void ComputeMissingNormals(std::vector<Vertex_PCUTBN>& verts, const std::vector<unsigned int>& indices);
void ComputeMissingTangentsBitangents(std::vector<Vertex_PCUTBN>& verts, const std::vector<unsigned int>& indices);
bool IsStringValidInteger(const std::string& s);