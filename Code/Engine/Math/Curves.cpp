#include "Curves.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"

CubicBezierCurve2D::CubicBezierCurve2D(Vec2 startPos, Vec2 guidePos1, Vec2 guidePos2, Vec2 endPos)
	: m_startPos(startPos), m_guidePos1(guidePos1), m_guidePos2(guidePos2), m_endPos(endPos)
{
}

CubicBezierCurve2D::CubicBezierCurve2D(CubicHermiteCurve2D const& fromHermite)
{
	m_startPos = fromHermite.m_startPos;
	m_endPos = fromHermite.m_endPos;

	const float k = 0.5f;

	m_guidePos1 = m_startPos + fromHermite.m_startVel * k;
	m_guidePos2 = m_endPos - fromHermite.m_endVel * k;
}


Vec2 CubicBezierCurve2D::EvaluateAtParametric(float t) const
{
	float oneMinusT = 1.0f - t;
	float oneMinusT2 = oneMinusT * oneMinusT;
	float oneMinusT3 = oneMinusT2 * oneMinusT;
	float t2 = t * t;
	float t3 = t2 * t;

	return oneMinusT3 * m_startPos +
		3.0f * oneMinusT2 * t * m_guidePos1 +
		3.0f * oneMinusT * t2 * m_guidePos2 +
		t3 * m_endPos;
}

float CubicBezierCurve2D::GetApproximateLength(int numSubdivisions) const
{
	float length = 0.0f;
	Vec2 prevPoint = EvaluateAtParametric(0.0f);
	for (int i = 1; i <= numSubdivisions; ++i)
	{
		float t = static_cast<float>(i) / numSubdivisions;
		Vec2 currentPoint = EvaluateAtParametric(t);
		length += (currentPoint - prevPoint).GetLength();
		prevPoint = currentPoint;
	}
	return length;
}

Vec2 CubicBezierCurve2D::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions) const
{
	float totalLength = GetApproximateLength(numSubdivisions);
	if (distanceAlongCurve <= 0.0f) return m_startPos;
	if (distanceAlongCurve >= totalLength) return m_endPos;

	float accumulatedLength = 0.0f;
	Vec2 prevPoint = m_startPos;

	for (int i = 1; i <= numSubdivisions; ++i)
	{
		float t = static_cast<float>(i) / numSubdivisions;
		Vec2 currentPoint = EvaluateAtParametric(t);
		float segmentLength = (currentPoint - prevPoint).GetLength();

		if (accumulatedLength + segmentLength >= distanceAlongCurve)
		{
			float ratio = (distanceAlongCurve - accumulatedLength) / segmentLength;
			return Interpolate(prevPoint, currentPoint, ratio);
		}

		accumulatedLength += segmentLength;
		prevPoint = currentPoint;
	}
	return m_endPos;
}

void CubicBezierCurve2D::AddVertsForBezierCurve(std::vector<Vertex_PCU>& verts, int subdivision, float thickness, Rgba8 const& color)
{
	std::vector<Vec2> curvePoints;
	curvePoints.reserve(subdivision + 1); 

	for (int i = 0; i <= subdivision; ++i) 
	{
		float t = (float)i / (float)subdivision;
		Vec2 point = EvaluateAtParametric(t);
		curvePoints.push_back(point);
	}

	for (int i = 0; i < (int)curvePoints.size() - 1; ++i) 
	{
		AddVertsForLineSegment2D(verts, curvePoints[i], curvePoints[i + 1], thickness, color);
	}
}


void CubicBezierCurve2D::AddVertsForGuidePoints(std::vector<Vertex_PCU>& verts, float radius, Rgba8 const& color)
{
	AddVertsForDisc2D(verts, m_startPos, radius, color);
	AddVertsForDisc2D(verts, m_guidePos1, radius, color);
	AddVertsForDisc2D(verts, m_guidePos2, radius, color);
	AddVertsForDisc2D(verts, m_endPos, radius, color);
}

void CubicBezierCurve2D::AddVertsForPoint(std::vector<Vertex_PCU>& verts, float t, float radius, Rgba8 const& color)
{
	Vec2 point = EvaluateAtParametric(t);
	AddVertsForDisc2D(verts, point, radius, color);
}

void CubicBezierCurve2D::AddVertsForGuideLines(std::vector<Vertex_PCU>& verts, float lineThickness, Rgba8 const& color)
{
	Vec2 A = m_startPos;
	Vec2 B = m_guidePos1;
	Vec2 C = m_guidePos2;
	Vec2 D = m_endPos;

	AddVertsForLineSegment2D(verts, A, B, lineThickness, color);
	AddVertsForLineSegment2D(verts, B, C, lineThickness, color);
	AddVertsForLineSegment2D(verts, C, D, lineThickness, color);
}













CubicHermiteCurve2D::CubicHermiteCurve2D(Vec2 startPos, Vec2 endPos, Vec2 startVel, Vec2 endVel)
	: m_startPos(startPos)
	, m_endPos(endPos)
	, m_startVel(startVel)
	, m_endVel(endVel)
{
}

CubicHermiteCurve2D::CubicHermiteCurve2D(CubicBezierCurve2D const& fromBezier)
{
	m_startPos = fromBezier.m_startPos;
	m_endPos = fromBezier.m_endPos;
	m_startVel = 3.f * (fromBezier.m_guidePos1 - fromBezier.m_startPos);
	m_endVel = 3.f * (fromBezier.m_endPos - fromBezier.m_guidePos2);
}

Vec2 CubicHermiteCurve2D::EvaluateAtParametric(float t) const
{
	float t2 = t * t;
	float t3 = t2 * t;

	float h00 = 2.f * t3 - 3.f * t2 + 1.f;
	float h10 = t3 - 2.f * t2 + t;
	float h01 = -2.f * t3 + 3.f * t2;
	float h11 = t3 - t2;

	return h00 * m_startPos + h10 * m_startVel + h01 * m_endPos + h11 * m_endVel;
}

float CubicHermiteCurve2D::GetApproximateLength(int numSubdivisions) const
{
	float length = 0.f;
	Vec2 prev = EvaluateAtParametric(0.f);

	for (int i = 1; i <= numSubdivisions; ++i)
	{
		float t = static_cast<float>(i) / static_cast<float>(numSubdivisions);
		Vec2 curr = EvaluateAtParametric(t);
		length += (curr - prev).GetLength();
		prev = curr;
	}

	return length;
}

Vec2 CubicHermiteCurve2D::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions) const
{
	if (distanceAlongCurve <= 0.f) return m_startPos;

	float totalLength = GetApproximateLength(numSubdivisions);
	if (distanceAlongCurve >= totalLength) return m_endPos;

	float accumulated = 0.f;
	Vec2 prev = EvaluateAtParametric(0.f);

	for (int i = 1; i <= numSubdivisions; ++i)
	{
		float t = static_cast<float>(i) / static_cast<float>(numSubdivisions);
		Vec2 curr = EvaluateAtParametric(t);
		float segmentLength = (curr - prev).GetLength();

		if (accumulated + segmentLength >= distanceAlongCurve)
		{
			float remaining = distanceAlongCurve - accumulated;
			float localT = remaining / segmentLength;
			return Interpolate(prev, curr, localT);
		}

		accumulated += segmentLength;
		prev = curr;
	}

	return m_endPos;
}

void CubicHermiteCurve2D::AddVertsForHermiteCurve(std::vector<Vertex_PCU>& verts, int subdivision, float thickness, Rgba8 const& color)
{
	Vec2 prevPos = EvaluateAtParametric(0.f);

	for (int i = 1; i <= subdivision; ++i)
	{
		float t = static_cast<float>(i) / static_cast<float>(subdivision);

		Vec2 curPos = EvaluateAtParametric(t);
		AddVertsForLineSegment2D(verts, prevPos, curPos, thickness, color);

		prevPos = curPos;
	}
}


CubicHermiteSpline2D::CubicHermiteSpline2D(std::vector<Vec2> const& positions)
	: m_positions(positions)
{
	int count = static_cast<int>(positions.size());

	m_velocities.resize(count);

	for (int i = 1; i < count - 1; ++i)
	{
		m_velocities[i] = (positions[i + 1] - positions[i - 1]) * 0.5f;
	}

	m_velocities[0] = Vec2::ZERO;          
	m_velocities[count - 1] = Vec2::ZERO;

	m_hermiteCurves.clear();
	for (int i = 0; i < count - 1; ++i)
	{
		CubicHermiteCurve2D curve(positions[i], positions[i + 1], m_velocities[i], m_velocities[i + 1]);
		m_hermiteCurves.push_back(curve);
	}
}

Vec2 CubicHermiteSpline2D::EvaluateAtParametric(float parametricZeroToOne) const
{
	int numSegments = static_cast<int>(m_hermiteCurves.size());
	if (numSegments <= 0) return Vec2();

	parametricZeroToOne = GetClamped(parametricZeroToOne, 0.f, 1.f);
	float scaledT = parametricZeroToOne * numSegments;

	int segmentIndex = static_cast<int>(scaledT);
	segmentIndex = (int) GetClamped((float) segmentIndex, 0, (float) numSegments - 1);

	float localT = scaledT - static_cast<float>(segmentIndex);

	return m_hermiteCurves[segmentIndex].EvaluateAtParametric(localT);
}

float CubicHermiteSpline2D::GetApproximateLength(int numSubdivisions) const
{
	float totalLength = 0.f;
	for (CubicHermiteCurve2D const& curve : m_hermiteCurves)
	{
		totalLength += curve.GetApproximateLength(numSubdivisions);
	}
	return totalLength;
}

Vec2 CubicHermiteSpline2D::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions) const
{
	float remainingDist = distanceAlongCurve;

	for (int i = 0; i < static_cast<int>(m_hermiteCurves.size()); ++i)
	{
		float segmentLength = m_hermiteCurves[i].GetApproximateLength(numSubdivisions);

		if (remainingDist <= segmentLength)
		{
			return m_hermiteCurves[i].EvaluateAtApproximateDistance(remainingDist, numSubdivisions);
		}
		else
		{
			remainingDist -= segmentLength;
		}
	}

	int last = static_cast<int>(m_hermiteCurves.size()) - 1;
	return m_hermiteCurves[last].EvaluateAtParametric(1.f);
}

void CubicHermiteSpline2D::AddVertsForHermiteSpline(std::vector<Vertex_PCU>& verts, int subdivision, float thickness, Rgba8 const& color)
{
	for (CubicHermiteCurve2D& curve : m_hermiteCurves)
	{
		curve.AddVertsForHermiteCurve(verts, subdivision, thickness, color);
	}
}



void CubicHermiteSpline2D::AddVertsForGuides(std::vector<Vertex_PCU>& verts, float radius, float thickness)
{
	for (Vec2 const& pos : m_positions)
	{
		AddVertsForDisc2D(verts, pos, radius, Rgba8(0, 120, 120));
	}

	for (int i = 0; i < static_cast<int>(m_positions.size()) - 1; ++i)
	{
		Vec2 const& start = m_positions[i];
		Vec2 const& end = m_positions[i + 1];
		AddVertsForLineSegment2D(verts, start, end, thickness * 0.3f, Rgba8(0, 100, 100));
	}

	Rgba8 tangentColor = Rgba8(255, 0, 0, 255);
	for (int i = 0; i < static_cast<int>(m_positions.size()); ++i)
	{
		Vec2 const& pos = m_positions[i];
		Vec2 const& vel = m_velocities[i];
		Vec2 arrowEnd = pos + vel * radius / 5.f;
		AddVertsForArrow2D(verts, pos, arrowEnd, radius * 1.5f, radius * 0.3f, Rgba8::RED);

	}
}





