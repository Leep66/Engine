#pragma once
#include "Engine/Math/Vec2.hpp"

struct LineSegment2
{
public:
	Vec2 m_start;
	Vec2 m_end;

public:
	LineSegment2() = default;
	LineSegment2(const Vec2& start, const Vec2& end);

	void Translate(Vec2 translation);
	void SetCenter(Vec2 newCenter);
	void RotateAboutCenter(float rotationDeltaDegrees);
};