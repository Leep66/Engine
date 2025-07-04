#include "Sphere3.hpp"

Sphere3::Sphere3(Sphere3 const& copyFrom)
	: m_center(copyFrom.m_center)
	, m_radius(copyFrom.m_radius)
{
}

Sphere3::Sphere3(Vec3 center, float radius)
	: m_center(center)
	, m_radius(radius)
{
}

void Sphere3::Translate(Vec3 const& translationToApply)
{
	m_center += translationToApply;
}

void Sphere3::SetCenter(Vec3 const& newCenter)
{
	m_center = newCenter;
}

Vec3 const Sphere3::GetCenter() const
{
	return m_center;
}
