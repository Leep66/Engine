#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"

struct OBB3
{
public:
	Vec3 m_iBasis;
	Vec3 m_jBasis;
	Vec3 m_kBasis;
	Vec3 m_halfDimensions;
	Vec3 m_center;
public:
	OBB3() = default;
	OBB3(OBB3 const& copyFrom);

	explicit OBB3(Vec3 i, Vec3 j, Vec3 k, Vec3 halfDimensions, Vec3 center);

	void Translate(Vec3 const& translationToApply);

	void SetCenter(Vec3 const& center);
	Vec3 const GetCenter() const;

	void Rotate(EulerAngles const& angle);
};