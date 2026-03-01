#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"

struct Plane2
{
	Vec2  m_normal;
	float m_dist;

	Plane2() = default;
	Plane2(Vec2 const& normal, float dist)
		: m_normal(normal), m_dist(dist) {
	}

	float GetSignedDistance(Vec2 const& p) const
	{
		return DotProduct2D(m_normal, p) - m_dist;
	}
};
