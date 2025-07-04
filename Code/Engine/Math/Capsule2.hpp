#pragma once
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/Vec2.hpp"

struct Capsule2
{
public:
	Vec2 m_start = Vec2::ZERO;
	Vec2 m_end = Vec2::ZERO;
	float m_radius = 0.f;

public:
	Capsule2() = default;
	Capsule2(const Vec2& start, const Vec2& end, float radius);
	Capsule2(const LineSegment2& bone, float radius);
	void Translate(Vec2 translation);
	void SetCenter(Vec2 newCenter);
	void RotateAboutCenter(float rotationDeltaDegrees);
};