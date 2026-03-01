#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec4.hpp"
#include <math.h>

const EulerAngles EulerAngles::ZERO(0.f, 0.f, 0.f);
EulerAngles::EulerAngles(float yawDegrees, float pitchDegrees, float rollDegrees)
	:m_yawDegrees(yawDegrees), m_pitchDegrees(pitchDegrees), m_rollDegrees(rollDegrees)
{
}

void EulerAngles::GetAsVectors_IFwd_JLeft_KUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis) const
{
	float yawRadians = ConvertDegreesToRadians(m_yawDegrees);
	float pitchRadians = ConvertDegreesToRadians(m_pitchDegrees);
	float rollRadians = ConvertDegreesToRadians(m_rollDegrees);

	float cy = cosf(yawRadians);
	float sy = sinf(yawRadians);
	float cp = cosf(pitchRadians);
	float sp = sinf(pitchRadians);
	float cr = cosf(rollRadians);
	float sr = sinf(rollRadians);

	out_forwardIBasis = Vec3(
		cy * cp,
		sy * cp,
		-sp
	);

	out_leftJBasis = Vec3(
		cy * sp * sr - sy * cr,
		sy * sp * sr + cy * cr,
		cp * sr
	);

	out_upKBasis = Vec3(
		cy * sp * cr + sy * sr,
		sy * sp * cr - cy * sr,
		cp * cr
	);
}


Mat44 EulerAngles::GetAsMatrix_IFwd_JLeft_KUp() const
{
	Mat44 matrix;
	matrix.AppendZRotation(m_yawDegrees);  
	matrix.AppendYRotation(m_pitchDegrees);
	matrix.AppendXRotation(m_rollDegrees);

	return matrix;
}

Vec3 EulerAngles::GetForwardNormal() const
{
	float yawRadians = ConvertDegreesToRadians(m_yawDegrees);
	float pitchRadians = ConvertDegreesToRadians(m_pitchDegrees);

	float cy = cosf(yawRadians);
	float sy = sinf(yawRadians);
	float cp = cosf(pitchRadians);
	float sp = sinf(pitchRadians);

	Vec3 out_forwardIBasis = Vec3(
		cy * cp,
		sy * cp,
		-sp
	);

	return out_forwardIBasis;
}

Vec3 EulerAngles::GetLeftNormal() const
{
	float yawRadians = ConvertDegreesToRadians(m_yawDegrees);
	float pitchRadians = ConvertDegreesToRadians(m_pitchDegrees);
	float rollRadians = ConvertDegreesToRadians(m_rollDegrees);

	float cy = cosf(yawRadians);
	float sy = sinf(yawRadians);
	float cp = cosf(pitchRadians);
	float sp = sinf(pitchRadians);
	float sr = sinf(rollRadians);
	
	float cr = cosf(rollRadians);
	Vec3 out_leftJBasis = Vec3(
		cy * sp * sr - sy * cr,
		sy * sp * sr + cy * cr,
		cp * sr
	);

	return out_leftJBasis;
}

Vec3 EulerAngles::GetUpNormal() const
{
	float yawRadians = ConvertDegreesToRadians(m_yawDegrees);
	float pitchRadians = ConvertDegreesToRadians(m_pitchDegrees);
	float rollRadians = ConvertDegreesToRadians(m_rollDegrees);

	float cy = cosf(yawRadians);
	float sy = sinf(yawRadians);
	float cp = cosf(pitchRadians);
	float sp = sinf(pitchRadians);
	float cr = cosf(rollRadians);
	float sr = sinf(rollRadians);

	Vec3 out_upKBasis = Vec3(
		cy * sp * cr + sy * sr,
		sy * sp * cr - cy * sr,
		cp * cr
	);

	return out_upKBasis;
}

EulerAngles EulerAngles::operator+(const EulerAngles& other) const
{
	return EulerAngles(
		m_yawDegrees + other.m_yawDegrees,
		m_pitchDegrees + other.m_pitchDegrees,
		m_rollDegrees + other.m_rollDegrees
	);
}

EulerAngles EulerAngles::operator-(const EulerAngles& other) const
{
	return EulerAngles(
		m_yawDegrees - other.m_yawDegrees,
		m_pitchDegrees - other.m_pitchDegrees,
		m_rollDegrees - other.m_rollDegrees
	);
}

EulerAngles EulerAngles::operator*(float scalar) const
{
	return EulerAngles(
		m_yawDegrees * scalar,
		m_pitchDegrees * scalar,
		m_rollDegrees * scalar
	);
}

EulerAngles EulerAngles::operator/(float scalar) const
{
	if (scalar == 0.f) 
	{
		return EulerAngles::ZERO;
	}
	return EulerAngles(
		m_yawDegrees / scalar,
		m_pitchDegrees / scalar,
		m_rollDegrees / scalar
	);
}

EulerAngles& EulerAngles::operator+=(const EulerAngles& other)
{
	m_yawDegrees += other.m_yawDegrees;
	m_pitchDegrees += other.m_pitchDegrees;
	m_rollDegrees += other.m_rollDegrees;
	return *this;
}

EulerAngles& EulerAngles::operator-=(const EulerAngles& other)
{
	m_yawDegrees -= other.m_yawDegrees;
	m_pitchDegrees -= other.m_pitchDegrees;
	m_rollDegrees -= other.m_rollDegrees;
	return *this;
}

EulerAngles& EulerAngles::operator*=(float scalar)
{
	m_yawDegrees *= scalar;
	m_pitchDegrees *= scalar;
	m_rollDegrees *= scalar;
	return *this;
}

EulerAngles& EulerAngles::operator/=(float scalar)
{
	if (scalar != 0.f) {
		m_yawDegrees /= scalar;
		m_pitchDegrees /= scalar;
		m_rollDegrees /= scalar;
	}
	return *this;
}

EulerAngles operator*(float scalar, const EulerAngles& angles)
{
	return angles * scalar;
}




