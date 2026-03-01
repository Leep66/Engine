#include "Engine/Core/StaticMeshUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec4.hpp"

#include <cstdio>

bool LoadStaticMeshFile(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices, std::string const& filePathNoExtension, Mat44 const& transform)
{
	std::string objFilePath = filePathNoExtension + ".obj";
	return LoadOBJMeshFile(verts, indices, objFilePath, transform);
}



bool LoadOBJMeshFile(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices, const std::string& objFilePath, Mat44 const& transform)
{
	FILE* file = nullptr;
	errno_t err = fopen_s(&file, objFilePath.c_str(), "r");
	if (err != 0 || file == nullptr)
	{
		return false;
	}

	std::vector<Vec3> positions;
	std::vector<Vec2> uvs;
	std::vector<Vec3> normals;

	std::map<std::string, unsigned int> vertexCache;

	char lineBuffer[1024];
	while (fgets(lineBuffer, sizeof(lineBuffer), file))
	{
		std::string line(lineBuffer);
		if (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
			line.pop_back();

		if (line.empty() || line[0] == '#')
			continue;

		std::vector<std::string> tokens = SplitStringOnDelimiter(line, ' ');
		if (tokens.empty())
			continue;

		if (tokens[0] == "v" && tokens.size() >= 4)
		{
			Vec3 pos{ std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]) };
			positions.push_back(pos);
		}
		else if (tokens[0] == "vt" && tokens.size() >= 3)
		{
			Vec2 uv{ std::stof(tokens[1]), std::stof(tokens[2]) };
			
			uvs.push_back(uv);
		}
		else if (tokens[0] == "vn" && tokens.size() >= 4)
		{
			Vec3 norm{ std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]) };
			normals.push_back(norm);
		}
		else if (tokens[0] == "f" && tokens.size() >= 4)
		{
			for (size_t tri = 2; tri < tokens.size() - 1; ++tri)
			{
				std::string vertStr[3] = { tokens[1], tokens[tri], tokens[tri + 1] };
				for (int vi = 0; vi < 3; ++vi)
				{
					std::string key = vertStr[vi];
					auto it = vertexCache.find(key);
					if (it != vertexCache.end())
					{
						indices.push_back(it->second);
					}
					else
					{
						std::vector<std::string> parts = SplitStringOnDelimiter(key, '/');
						int vIdx = (parts.size() > 0) ? std::stoi(parts[0]) : 0;
						int vtIdx = (parts.size() > 1 && !parts[1].empty()) ? std::stoi(parts[1]) : 0;
						int vnIdx = (parts.size() > 2) ? std::stoi(parts[2]) : 0;

						Vertex_PCUTBN vert;
						if (vIdx > 0 && vIdx <= (int)positions.size())
							vert.m_position = positions[vIdx - 1];
						else
							vert.m_position = Vec3{ 0,0,0 };

						if (vtIdx > 0 && vtIdx <= (int)uvs.size())
							vert.m_uvTexCoords = uvs[vtIdx - 1];
						else
							vert.m_uvTexCoords = Vec2{ 0,0 };

						if (vnIdx > 0 && vnIdx <= (int)normals.size())
							vert.m_normal = normals[vnIdx - 1];
						else
							vert.m_normal = Vec3{ 0,0,0 };

						vert.m_color = Rgba8::WHITE;

						verts.push_back(vert);
						unsigned int newIndex = static_cast<unsigned int>(verts.size() - 1);
						indices.push_back(newIndex);
						vertexCache[key] = newIndex;
					}
				}
			}
		}
	}

	fclose(file);

	ComputeMissingNormals(verts, indices);
	ComputeMissingTangentsBitangents(verts, indices);

	for (auto& vert : verts)
	{
		vert.m_position = transform.TransformPosition3D(vert.m_position);
		vert.m_normal = transform.TransformVectorQuantity3D(vert.m_normal).GetNormalized();
		vert.m_tangent = transform.TransformVectorQuantity3D(vert.m_tangent).GetNormalized();
		vert.m_bitangent = transform.TransformVectorQuantity3D(vert.m_bitangent).GetNormalized();
	}

	return true;
}






void ComputeMissingNormals(std::vector<Vertex_PCUTBN>& verts, const std::vector<unsigned int>& indices)
{
	if (indices.size() % 3 != 0)
	{
		return;
	}

	

	for (size_t i = 0; i < indices.size(); i += 3)
	{
		Vertex_PCUTBN& v0 = verts[indices[i]];
		Vertex_PCUTBN& v1 = verts[indices[i + 1]];
		Vertex_PCUTBN& v2 = verts[indices[i + 2]];

		bool hasNormal0 = (v0.m_normal.x != 0.f || v0.m_normal.y != 0.f || v0.m_normal.z != 0.f);
		bool hasNormal1 = (v1.m_normal.x != 0.f || v1.m_normal.y != 0.f || v1.m_normal.z != 0.f);
		bool hasNormal2 = (v2.m_normal.x != 0.f || v2.m_normal.y != 0.f || v2.m_normal.z != 0.f);
		if (hasNormal0 && hasNormal1 && hasNormal2)
			continue;

		Vec3 edge1 = v1.m_position - v0.m_position;
		Vec3 edge2 = v2.m_position - v0.m_position;
		Vec3 faceNormal = CrossProduct3D(edge1, edge2).GetNormalized();

		if (!hasNormal0) v0.m_normal = faceNormal;
		if (!hasNormal1) v1.m_normal = faceNormal;
		if (!hasNormal2) v2.m_normal = faceNormal;
	}
}

void ComputeMissingTangentsBitangents(std::vector<Vertex_PCUTBN>& vertices, const std::vector<unsigned int>& indices)
{
	// 1. ???????
	for (size_t i = 0; i + 2 < indices.size(); i += 3)
	{
		unsigned int i0 = indices[i + 0];
		unsigned int i1 = indices[i + 1];
		unsigned int i2 = indices[i + 2];

		Vertex_PCUTBN& v0 = vertices[i0];
		Vertex_PCUTBN& v1 = vertices[i1];
		Vertex_PCUTBN& v2 = vertices[i2];

		// 1. E0 = P1 - P0
		Vec3 E0 = v1.m_position - v0.m_position;
		// 2. E1 = P2 - P0
		Vec3 E1 = v2.m_position - v0.m_position;

		// 4. ?? ?u ? ?v
		float deltaU0 = v1.m_uvTexCoords.x - v0.m_uvTexCoords.x;
		float deltaU1 = v2.m_uvTexCoords.x - v0.m_uvTexCoords.x;
		float deltaV0 = v1.m_uvTexCoords.y - v0.m_uvTexCoords.y;
		float deltaV1 = v2.m_uvTexCoords.y - v0.m_uvTexCoords.y;

		// 5. ?? r
		float r = (deltaU0 * deltaV1 - deltaU1 * deltaV0);
		if (r != 0.f)
			r = 1.0f / r;
		else
			r = 0.0f; // ????

		// 6. T = r * (?v1 * E0 - ?v0 * E1)
		Vec3 T = (E0 * deltaV1 - E1 * deltaV0) * r;
		// 7. B = r * (?u0 * E1 - ?u1 * E0)
		Vec3 B = (E1 * deltaU0 - E0 * deltaU1) * r;

		// ???????
		v0.m_tangent += T;
		v1.m_tangent += T;
		v2.m_tangent += T;

		v0.m_bitangent += B;
		v1.m_bitangent += B;
		v2.m_bitangent += B;

		// ???????? (??)
		Vec3 N = CrossProduct3D(E0, E1).GetNormalized();
		v0.m_normal += N;
		v1.m_normal += N;
		v2.m_normal += N;
	}

	// 3. ??????????? Gram-Schmidt ???
	for (Vertex_PCUTBN& v : vertices)
	{
		v.m_normal = v.m_normal.GetNormalized();
		v.m_tangent = v.m_tangent.GetNormalized();
		v.m_bitangent = v.m_bitangent.GetNormalized();

		// Gram-Schmidt ??? T ? N
		v.m_tangent = (v.m_tangent - v.m_normal * DotProduct3D(v.m_normal, v.m_tangent)).GetNormalized();

		// ?? bitangent ?????????????? Cross(N,T)
		v.m_bitangent = CrossProduct3D(v.m_normal, v.m_tangent);
		v.m_bitangent = v.m_bitangent.GetNormalized();
	}
}


bool IsStringValidInteger(const std::string& s) 
{
	if (s.empty()) return false;

	size_t i = 0;
	if (s[0] == '-' || s[0] == '+') {
		i++;
		if (i == s.size()) return false; 
	}

	for (; i < s.size(); ++i) {
		if (!isdigit(static_cast<unsigned char>(s[i]))) {
			return false;
		}
	}

	return true;
}

