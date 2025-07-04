#include "Triangle2.hpp"

Triangle2::Triangle2(Vec2 ccw0, Vec2 ccw1, Vec2 ccw2)
{
	m_pointsCounterClockwise[0] = ccw0;
	m_pointsCounterClockwise[1] = ccw1;
	m_pointsCounterClockwise[2] = ccw2;
}

void Triangle2::Translate(Vec2 translation)
{
	Vec2 triangleVertA = m_pointsCounterClockwise[0];
	Vec2 triangleVertB = m_pointsCounterClockwise[1];
	Vec2 triangleVertC = m_pointsCounterClockwise[2];

	triangleVertA += translation;
	triangleVertB += translation;
	triangleVertC += translation;
}
