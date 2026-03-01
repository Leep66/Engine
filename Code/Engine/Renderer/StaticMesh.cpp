#include "Engine/Renderer/StaticMesh.hpp"
#include "Engine/Core/StaticMeshUtils.hpp"

StaticMesh::StaticMesh(std::string name, const char* path)
	: m_name(name)
{
	m_meshDef = StaticMeshDefinition::GetDefinition(m_name);

	std::string xStr = m_meshDef.m_xAxis;
	std::string yStr = m_meshDef.m_yAxis;
	std::string zStr = m_meshDef.m_zAxis;

	Vec3 xAxis = StringToAxisVector(xStr);
	Vec3 yAxis = StringToAxisVector(yStr);
	Vec3 zAxis = StringToAxisVector(zStr);

	Mat44 transform;
	transform.SetIJK3D(xAxis, yAxis, zAxis);

	LoadStaticMeshFile(m_vertices, m_indices, path, transform);
}

StaticMesh::StaticMesh()
{
	
}

StaticMesh::~StaticMesh()
{

}

Vec3 StaticMesh::StringToAxisVector(std::string str)
{
	Vec3 Axis;
	if (str == "left") Axis = Vec3(0.f, 1.f, 0.f);
	else if (str == "right") Axis = Vec3(0.f, -1.f, 0.f);
	else if (str == "up")    Axis = Vec3(0.f, 0.f, 1.f);
	else if (str == "down")  Axis = Vec3(0.f, 0.f, -1.f);
	else if (str == "forward") Axis = Vec3(1.f, 0.f, 0.f);
	else if (str == "back")    Axis = Vec3(-1.f, 0.f, 0.f);
	else Axis = Vec3::ZERO;

	return Axis;
}

