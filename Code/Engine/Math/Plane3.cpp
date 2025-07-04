#include "Plane3.hpp"
#include "Engine/Math/MathUtils.hpp"

Plane3::Plane3(Plane3 const& copyFrom)
	: m_normal(copyFrom.m_normal)
	, m_distFromOrigin(copyFrom.m_distFromOrigin)
{
}

Plane3::Plane3(Vec3 normal, float distToOrigin)
	: m_normal(normal)
	, m_distFromOrigin(distToOrigin)
{
}

void Plane3::Translate(Vec3 const& translationToApply)
{
	float offset = DotProduct3D(translationToApply, m_normal);
	m_distFromOrigin += offset;
}