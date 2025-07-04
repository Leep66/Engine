#include "OBB3.hpp"
#include "Engine/Math/Mat44.hpp"


OBB3::OBB3(OBB3 const& copyFrom)
	: m_iBasis(copyFrom.m_iBasis)
	, m_jBasis(copyFrom.m_jBasis)
	, m_kBasis(copyFrom.m_kBasis)
	, m_halfDimensions(copyFrom.m_halfDimensions)
	, m_center(copyFrom.m_center)
{
}

OBB3::OBB3(Vec3 i, Vec3 j, Vec3 k, Vec3 halfDimensions, Vec3 center)
	: m_iBasis(i)
	, m_jBasis(j)
	, m_kBasis(k)
	, m_halfDimensions(halfDimensions)
	, m_center(center)
{
}



void OBB3::Translate(Vec3 const& translationToApply)
{
	m_center += translationToApply;
}

void OBB3::SetCenter(Vec3 const& center)
{
	m_center = center;
}

Vec3 const OBB3::GetCenter() const
{
	return m_center;
}

void OBB3::Rotate(EulerAngles const& angle)
{
	Mat44 rotationMatrix;
	rotationMatrix.AppendZRotation(angle.m_yawDegrees);
	rotationMatrix.AppendYRotation(angle.m_pitchDegrees);
	rotationMatrix.AppendXRotation(angle.m_rollDegrees);


	m_iBasis = rotationMatrix.TransformVectorQuantity3D(m_iBasis);
	m_jBasis = rotationMatrix.TransformVectorQuantity3D(m_jBasis);
	m_kBasis = rotationMatrix.TransformVectorQuantity3D(m_kBasis);
}
