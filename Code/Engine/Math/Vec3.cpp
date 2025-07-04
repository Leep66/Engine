#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include <math.h>

const Vec3 Vec3::ZERO(0.f, 0.f, 0.f);
const Vec3 Vec3::ONE(1.f, 1.f, 1.f);


Vec3::Vec3(float initialX, float initialY, float initialZ)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
{
}

Vec3::Vec3(Vec2 xy, float z)
{
	x = xy.x;
	y = xy.y;
	z = z;
}

float Vec3::GetLength() const
{
	return sqrtf((x * x) + (y * y) + (z * z));
}

float Vec3::GetLengthXY() const
{
	return sqrtf((x * x) + (y * y));
}

Vec2 Vec3::GetXY() const
{
	return Vec2(x, y);
}

Vec3 Vec3::IgnoreZ() const
{
	return Vec3(x, y, 0.f);
}

float Vec3::GetLengthSquared() const
{
	return ((x * x) + (y * y) + (z * z));
}

float Vec3::GetLengthXYSquared() const
{
	return ((x * x) + (y * y));
}

float Vec3::GetAngleAboutZRadians() const
{
	return atan2f(y, x);
}

float Vec3::GetAngleAboutZDegrees() const
{
	return Atan2Degrees(y, x);
}

Vec3 const Vec3::GetRotatedAboutZRadians(float deltaRadians) const
{
	float oldLen = sqrtf(x * x + y * y);
	float radians = atan2f(y, x);

	float newRadians = radians + deltaRadians;

	return Vec3(oldLen * cosf(newRadians), oldLen * sinf(newRadians), z);
}

Vec3 const Vec3::GetRotatedAboutZDegrees(float deltaDegrees) const
{
	float oldLen = sqrtf(x * x + y * y);
	float radians = atan2f(y, x);

	float newRadians = radians + ConvertDegreesToRadians(deltaDegrees);

	return Vec3(oldLen * cosf(newRadians), oldLen * sinf(newRadians), z);
}

Vec3 const Vec3::GetClamped(float maxLength) const
{
	float oldLen = GetLength();
	float uniformScale = 1;

	if (oldLen > maxLength)
	{
		uniformScale = maxLength / oldLen;
	}

	return Vec3(x * uniformScale, y * uniformScale, z * uniformScale);
}

Vec3 const Vec3::GetClamped(float min, float max) const
{
	return Vec3(
		(x < min) ? min : ((x > max) ? max : x),
		(y < min) ? min : ((y > max) ? max : y),
		(z < min) ? min : ((z > max) ? max : z)
	);
}

Vec3 const Vec3::GetNormalized() const
{
	float oldLen = GetLength();

	float uniformScale = 1 / oldLen;

	return Vec3(x * uniformScale, y * uniformScale, z * uniformScale);
}

EulerAngles Vec3::GetAsEulerAngles() const
{
	EulerAngles angles;

	angles.m_yawDegrees = Atan2Degrees(y, x);

	float horizontalLength = sqrtf(x * x + y * y);
	angles.m_pitchDegrees = Atan2Degrees(z, horizontalLength);

	angles.m_rollDegrees = 0.f;

	return angles;
}

Vec3 const Vec3::MakeFromPolarDegrees(float orientationDegrees, float zDegrees, float length)
{
	float polarRad = ConvertDegreesToRadians(orientationDegrees);
	float azimuthRad = ConvertDegreesToRadians(zDegrees);

	float x = length * sinf(polarRad) * cosf(azimuthRad);
	float y = length * sinf(polarRad) * sinf(azimuthRad);
	float z = length * cosf(polarRad);

	return Vec3(x, y, z);
}

Rgba8 Vec3::ToRgba()
{
	return Rgba8(
		static_cast<unsigned char>(GetClamped(0.f, 1.f).x * 255.f),
		static_cast<unsigned char>(GetClamped(0.f, 1.f).y * 255.f),
		static_cast<unsigned char>(GetClamped(0.f, 1.f).z * 255.f),
		255
	);
}

bool Vec3::operator==(Vec3 const& compare) const
{
	return (x == compare.x && y == compare.y && z == compare.z );
}

bool Vec3::operator!=(Vec3 const& compare) const
{
	return (x != compare.x || y != compare.y || z != compare.z);
}

Vec3 const Vec3::operator+(Vec3 const& vecToAdd) const
{
	return Vec3(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z);
}

Vec3 const Vec3::operator-(Vec3 const& vecToSubtract) const
{
	return Vec3(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z);
}

Vec3 const Vec3::operator-() const
{
	return Vec3(-x, -y, -z);
}

Vec3 const Vec3::operator*(float uniformScale) const
{
	return Vec3(x * uniformScale, y * uniformScale, z * uniformScale);
}

Vec3 const Vec3::operator*(Vec3 const& vecToMultiply) const
{
	return Vec3(x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z);
}

Vec3 const Vec3::operator/(float inverseScale) const
{
	return Vec3(x / inverseScale, y / inverseScale, z / inverseScale);
}

void Vec3::operator+=(Vec3 const& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}

void Vec3::operator-=(Vec3 const& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}

void Vec3::operator*=(const float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}

void Vec3::operator/=(const float uniformDivisor)
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
}

void Vec3::operator=(Vec3 const& copyFrom)	
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

Vec3 const operator*(float uniformScale, Vec3 const& vecToScale)
{
	return Vec3(vecToScale.x * uniformScale, vecToScale.y * uniformScale, vecToScale.z * uniformScale);
}
