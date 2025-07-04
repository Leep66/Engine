#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>

class CubicBezierCurve2D;
class CubicHermiteCurve2D;

class CubicBezierCurve2D
{
public:
	CubicBezierCurve2D(Vec2 startPos, Vec2 guidePos1, Vec2 guidePos2, Vec2 endPos);
	explicit CubicBezierCurve2D(CubicHermiteCurve2D const& fromHermite);
	Vec2 EvaluateAtParametric(float t) const;
	float GetApproximateLength(int numSubdivisions = 64) const;
	Vec2 EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions = 64) const;

	void AddVertsForBezierCurve(std::vector<Vertex_PCU>& verts, int subdivision, float thickness, Rgba8 const& color);
	void AddVertsForGuidePoints(std::vector<Vertex_PCU>& verts, float radius, Rgba8 const& color);
	void AddVertsForPoint(std::vector<Vertex_PCU>& verts, float t, float pointRadius, Rgba8 const& color);

	void AddVertsForGuideLines(std::vector<Vertex_PCU>& verts, float lineThickness, Rgba8 const& color);


public:
	Vec2 m_startPos;
	Vec2 m_guidePos1;
	Vec2 m_guidePos2;
	Vec2 m_endPos;

};










class CubicHermiteCurve2D
{
public:
	CubicHermiteCurve2D(Vec2 startPos, Vec2 endPos, Vec2 startVel, Vec2 endVel);
	explicit CubicHermiteCurve2D(CubicBezierCurve2D const& fromBezier);
	Vec2 EvaluateAtParametric(float parametricZeroToOne) const;
	float GetApproximateLength(int numSubdivisions = 64) const;
	Vec2 EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions = 64) const;

	void AddVertsForHermiteCurve(std::vector<Vertex_PCU>& verts, int subdivision, float thickness = 3.f, Rgba8 const& color = Rgba8(0, 200, 0));

public:
	Vec2 m_startPos;
	Vec2 m_endPos;
	Vec2 m_startVel;
	Vec2 m_endVel;
};









class CubicHermiteSpline2D
{
public:
	CubicHermiteSpline2D() = default;
	CubicHermiteSpline2D(std::vector<Vec2> const& positions);

	Vec2 EvaluateAtParametric(float parametricZeroToOne) const;
	float GetApproximateLength(int numSubdivisions = 64) const;
	Vec2 EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions = 64) const;

	void AddVertsForHermiteSpline(std::vector<Vertex_PCU>& verts, int subdivision, float thickness, Rgba8 const& color);
	void AddVertsForGuides(std::vector<Vertex_PCU>& verts, float radius, float thickness);

public:
	std::vector<Vec2> m_positions;
	std::vector<Vec2> m_velocities;
	std::vector<CubicHermiteCurve2D> m_hermiteCurves;
};