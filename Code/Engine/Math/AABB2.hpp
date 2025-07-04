#pragma once

#include "Engine/Math/Vec2.hpp"

struct AABB2
{
public:
	Vec2 m_mins;
	Vec2 m_maxs;

	static const AABB2 ZERO;
	static const AABB2 ZERO_TO_ONE;

public:
	~AABB2() {}
	AABB2() {}
	AABB2(AABB2 const& copyFrom);
	explicit AABB2(float minX, float minY, float maxX, float maxY);
	explicit AABB2(Vec2 const& mins, Vec2 const& maxs);

	bool IsPointInside(Vec2 const& point) const;
	Vec2 const GetCenter() const;
	Vec2 const GetDimensions() const;
	float GetAspect() const;
	Vec2 const GetNearestPoint(Vec2 const& referencePosition) const;
	Vec2 const GetPointAtUV(Vec2 const& uv) const;
	Vec2 const GetUVForPoint(Vec2 const& point) const;
	AABB2 GetBoxAtUVs(Vec2 uvMins, Vec2 uvMaxs) const;

	void Translate(Vec2 const& translationToApply);
	void SetCenter(Vec2 const& newCenter);
	void SetDimensions(Vec2 const& newDimensions);
	void StretchToIncludePoint(Vec2 const& point);
	void ClampWithin(AABB2 const& containingBox);
	void AddPadding(float xToaddOnBothSides, float yToAddToTopAndBottom);
	void ReduceToAspect(float newAspect);
	void EnlargeToAspect(float newAspect);

	AABB2 ChopOffTop(float percentOfOriginalToChop, float extraHeightOfOriginalToChop);
	AABB2 ChopOffBottom(float percentOfOriginalToChop, float extraHeightOfOriginalToChop);
	AABB2 ChopOffLeft(float percentOfOriginalToChop, float extraWidthOfOriginalToChop);
	AABB2 ChopOffRight(float percentOfOriginalToChop, float extraWidthOfOriginalToChop);

	// operators
	bool operator==(AABB2 const& rightHandSideCompare) const;
};