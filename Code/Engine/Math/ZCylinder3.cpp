#include "Engine/Math/ZCylinder3.hpp"

ZCylinder3::ZCylinder3(Vec2 centerXY, float minZ, float maxZ, float radius)
	: m_centerXY(centerXY)
	, m_minZ(minZ)
	, m_maxZ(maxZ)
	, m_radius(radius) 
{
}

void ZCylinder3::Translation(Vec3 const& translation)
{
	m_centerXY += Vec2(translation.x, translation.y);
	m_minZ += translation.z;
	m_maxZ += translation.z;
}

void ZCylinder3::SetCenter(Vec3 const& newCenter)
{
	Vec3 currentCenter = GetCenter();
	Vec3 offset = newCenter - currentCenter;

	m_centerXY += Vec2(offset.x, offset.y);
	m_minZ += offset.z;
	m_maxZ += offset.z;
}

Vec3 const ZCylinder3::GetCenter() const
{
	float centerZ = (m_minZ + m_maxZ) * 0.5f;
	return Vec3(m_centerXY.x, m_centerXY.y, centerZ);
}

