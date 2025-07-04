#include "LineSegment2.hpp"
#include "Engine/Math/MathUtils.hpp"

LineSegment2::LineSegment2(const Vec2& start, const Vec2& end)
	:m_start(start), m_end(end)
{
}

void LineSegment2::Translate(Vec2 translation)
{
	m_start += translation;
	m_end += translation;
}

void LineSegment2::SetCenter(Vec2 newCenter)
{
	Vec2 currentCenter = (m_end - m_start) / 2;

	Vec2 translation = newCenter - currentCenter;

	Translate(translation);
}

void LineSegment2::RotateAboutCenter(float rotationDeltaDegrees)
{
	Vec2 center = (m_start + m_end) * 0.5f;
	
	float cosTheta = CosDegrees(rotationDeltaDegrees);
	float sinTheta = SinDegrees(rotationDeltaDegrees);

	Vec2 translatedStart = m_start - center;
	float rotatedStartX = translatedStart.x * cosTheta - translatedStart.y * sinTheta;
	float rotatedStartY = translatedStart.x * sinTheta + translatedStart.y * cosTheta;
	m_start = Vec2(rotatedStartX, rotatedStartY) + center;

	Vec2 translatedEnd = m_end - center;
	float rotatedEndX = translatedEnd.x * cosTheta - translatedEnd.y * sinTheta;
	float rotatedEndY = translatedEnd.x * sinTheta + translatedEnd.y * cosTheta;
	m_end = Vec2(rotatedEndX, rotatedEndY) + center;

}

