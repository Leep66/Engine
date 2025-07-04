#pragma once
#include "Engine/Math/Vec3.hpp"

struct Sphere3
{
public:
	Vec3 m_center;
	float m_radius = 0.f;

public:
	Sphere3() {};
	Sphere3(Sphere3 const& copyFrom);

	explicit Sphere3(Vec3 center, float radius);

	void Translate(Vec3 const& translationToApply);
	void SetCenter(Vec3 const& newCenter);

	Vec3 const GetCenter() const;
};