#pragma once
#include "Engine/Math/Vec2.hpp"
struct OBB2
{
public:
	Vec2 m_center;
	Vec2 m_iBasisNormal;
	Vec2 m_halfDimensions;

public:
	OBB2() = default;
	OBB2(const Vec2& center, const Vec2& iBasisNormal, const Vec2& halfDimensions);

	void GetCornerPoints(Vec2* out_fourCornerWorldPositions) const;
	Vec2 const GetLocalPosForWorldPos(Vec2 const& worldPos) const;
	Vec2 const GetWorldPosForLocalPos(Vec2 const& localPos) const;
	void RotateAboutCenter(float rotationDeltaDegrees);
};