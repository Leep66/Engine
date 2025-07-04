#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"

struct Plane3
{
	Vec3 m_normal;
	float m_distFromOrigin;

	Plane3() = default;
	Plane3(Plane3 const& copyFrom);
	explicit Plane3(Vec3 normal, float distToOrigin);

	void Translate(Vec3 const& translationToApply);

	Vec3 GetCenter() const {
		return m_normal * m_distFromOrigin;
	}

	void SetCenter(const Vec3& newCenter) {
		m_distFromOrigin = DotProduct3D(m_normal, newCenter);
	}
};