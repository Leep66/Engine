#include "AABB3.hpp"

AABB3::AABB3(AABB3 const& copyFrom)
{
	m_mins = copyFrom.m_mins;
	m_maxs = copyFrom.m_maxs;
}

AABB3::AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
{
	m_mins = Vec3(minX, minY, minZ);
	m_maxs = Vec3(maxX, maxY, maxZ);
}

AABB3::AABB3(Vec3 const& mins, Vec3 const& maxs)
{
	m_mins = mins;
	m_maxs = maxs;
}

void AABB3::Translate(Vec3 const& translationToApply)
{
	m_mins += translationToApply;
	m_maxs += translationToApply;
}

void AABB3::SetCenter(Vec3 const& newCenter)
{
	Vec3 halfExtents = (m_maxs - m_mins) * 0.5f;
	m_mins = newCenter - halfExtents;
	m_maxs = newCenter + halfExtents;
}

Vec3 const AABB3::GetCenter() const
{
	return (m_mins + m_maxs) * 0.5f;
}

