#pragma once
#include "Engine/Math/Vec2.hpp"

struct Triangle2
{
public:
	Vec2 m_pointsCounterClockwise[3];

public:
	Triangle2() = default;
	Triangle2(Vec2 ccw0, Vec2 ccw1, Vec2 ccw2);
	void Translate(Vec2 translation);
};