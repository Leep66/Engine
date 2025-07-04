#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <math.h>

Vec4::Vec4(float initialX, float initialY, float initialZ, float initialW)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
	, w(initialW)
{
}

float Vec4::GetLength() const
{
	return sqrtf((x * x) + (y * y) + (z * z) + (w * w));
}

float Vec4::GetLengthXY() const
{
	return sqrtf((x * x) + (y * y));
}

float Vec4::GetLengthSquared() const
{
	return (x * x) + (y * y) + (z * z) + (w * w);
}

float Vec4::GetLengthXYSquared() const
{
	return ((x * x) + (y * y));
}

Vec4 const Vec4::GetClamped(float maxLength) const
{
	float oldLen = GetLength();
	float uniformScale = 1;

	if (oldLen > maxLength)
	{
		uniformScale = maxLength / oldLen;
	}

	return Vec4(x * uniformScale, y * uniformScale, z * uniformScale, w * uniformScale);
}

Vec4 const Vec4::GetNormalized() const
{
	float oldLen = GetLength();

	float uniformScale = 1 / oldLen;

	return Vec4(x * uniformScale, y * uniformScale, z * uniformScale, w * uniformScale);
}

Vec4 const Vec4::GetClamped(float min, float max) const
{
	return Vec4(
		(x < min) ? min : ((x > max) ? max : x),
		(y < min) ? min : ((y > max) ? max : y),
		(z < min) ? min : ((z > max) ? max : z),
		(w < min) ? min : ((w > max) ? max : w)
	);
}

Rgba8 Vec4::ToRgba()
{
	return Rgba8(
		static_cast<unsigned char>(GetClamped(0.f, 1.f).x * 255.f),
		static_cast<unsigned char>(GetClamped(0.f, 1.f).y * 255.f),
		static_cast<unsigned char>(GetClamped(0.f, 1.f).z * 255.f),
		static_cast<unsigned char>(GetClamped(0.f, 1.f).w * 255.f)
	);
}

bool Vec4::operator==(Vec4 const& compare) const
{
	return (x == compare.x && y == compare.y && z == compare.z && w == compare.w);
}

bool Vec4::operator!=(Vec4 const& compare) const
{
	return (x != compare.x || y != compare.y || z != compare.z || w != compare.w);
}

Vec4 const Vec4::operator+(Vec4 const& vecToAdd) const
{
	return Vec4(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z, w + vecToAdd.w);
}

Vec4 const Vec4::operator-(Vec4 const& vecToSubtract) const
{
	return Vec4(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z, w - vecToSubtract.w);
}

Vec4 const Vec4::operator-() const
{
	return Vec4(-x, -y, -z, -w);
}

Vec4 const Vec4::operator*(float uniformScale) const
{
	return Vec4(x * uniformScale, y * uniformScale, z * uniformScale, w * uniformScale);
}

Vec4 const Vec4::operator*(Vec4 const& vecToMultiply) const
{
	return Vec4(x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z, w * vecToMultiply.w);
}

Vec4 const Vec4::operator/(float inverseScale) const
{
	return Vec4(x / inverseScale, y / inverseScale, z / inverseScale, w / inverseScale);
}

void Vec4::operator+=(Vec4 const& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
	w += vecToAdd.w;
}

void Vec4::operator-=(Vec4 const& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
	w -= vecToSubtract.w;
}

void Vec4::operator*=(const float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
	w *= uniformScale;
}

void Vec4::operator/=(const float uniformDivisor)
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
	w /= uniformDivisor;
}

void Vec4::operator=(Vec4 const& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
	w = copyFrom.w;
}

Vec4 const operator*(float uniformScale, Vec4 const& vecToScale)
{
	return Vec4(vecToScale.x * uniformScale, vecToScale.y * uniformScale, vecToScale.z * uniformScale, vecToScale.w * uniformScale);
}
