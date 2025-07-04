#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

struct ZCylinder3
{
public:
	Vec2 m_centerXY = Vec2::ZERO;
	float m_minZ = 0.f;
	float m_maxZ = 0.f;
	float m_radius = 0.f;

public:
	ZCylinder3() = default;
	ZCylinder3(Vec2 centerXY, float minZ, float maxZ, float radius);

	void Translation(Vec3 const& translation);
	void SetCenter(Vec3 const& newCenter);
	
	Vec3 const GetCenter() const;

};