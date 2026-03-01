#pragma once
#include <vector>
#include <string>
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Renderer/StaticMeshDefinition.hpp"

class StaticMesh
{
public:
	StaticMesh(std::string name, const char* path);
	StaticMesh();
	~StaticMesh();

	Vec3 StringToAxisVector(std::string str);
	void SetName(std::string name) { m_name = name; }
	void SetVertsAndIndices(std::vector<Vertex_PCUTBN> verts, std::vector<unsigned int> indices) 
	{ m_vertices = verts; m_indices = indices; }

public:
	std::string m_name;
	std::vector<Vertex_PCUTBN> m_vertices;
	std::vector<unsigned int> m_indices;
	StaticMeshDefinition m_meshDef;
};