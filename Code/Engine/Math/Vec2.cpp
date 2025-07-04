#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
//#include "Engine/Core/EngineCommon.hpp"
#include <math.h>
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"

const Vec2 Vec2::ZERO(0, 0);
const Vec2 Vec2::ONE(1, 1);
//-----------------------------------------------------------------------------------------------
Vec2::Vec2( Vec2 const& copy )
	: x( copy.x )
	, y( copy.y )
{
}


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x( initialX )
	, y( initialY )
{
}

Vec2 const Vec2::MakeFromPolarRadians(float orientationRadians, float length)
{
	float x = length * cosf(orientationRadians);
	float y = length * sinf(orientationRadians);

	return Vec2(x, y);
}

Vec2 const Vec2::MakeFromPolarDegrees(float orientationDegrees, float length)
{
	float radians = ConvertDegreesToRadians(orientationDegrees);

	float x = length * cosf(radians);
	float y = length * sinf(radians);

	return Vec2(x, y);
}

float Vec2::GetLength() const
{
	return sqrtf((x * x) + (y * y));
}

float Vec2::GetLengthSquared() const
{
	return ((x * x) + (y * y));
}

float Vec2::GetOrientationRadians() const
{
	return atan2f(y, x);
}

float Vec2::GetOrientationDegrees() const
{
	return Atan2Degrees(y, x);
}

Vec2 const Vec2::GetRotated90Degrees() const
{
	return Vec2(-y, x);
}

Vec2 const Vec2::GetRotatedMinus90Degrees() const
{
	return Vec2(y, -x);
}

Vec2 const Vec2::GetRotatedRadians(float deltaRadians) const
{
	float oldLen = GetLength();

	float orientationRadians = atan2f(y, x);
	
	float newRadians = orientationRadians + deltaRadians;

	return Vec2((oldLen * cosf(newRadians)), (oldLen * sinf(newRadians)));
}

Vec2 const Vec2::GetRotatedDegrees(float deltaDegrees) const
{
	float oldLen = GetLength();

	float orientationDegrees = Atan2Degrees(y, x);
	
	float newRadians = ConvertDegreesToRadians(orientationDegrees + deltaDegrees);

	return Vec2((oldLen * cosf(newRadians)), (oldLen * sinf(newRadians)));
}

Vec2 const Vec2::GetClamped(float maxLength) const
{
	float oldLen = GetLength();

	if (oldLen <= maxLength) return *this;

	float uniformScale = maxLength / oldLen;
	return Vec2(x * uniformScale, y * uniformScale);

}

Vec2 const Vec2::GetNormalized() const
{
	float oldLen = GetLength();

	float uniformScale = 1 / oldLen;
	return Vec2(x * uniformScale, y * uniformScale);
}

IntVec2 const Vec2::ToIntVec2() const
{
	return IntVec2((int)x, (int)y);
}

void Vec2::SetOrientationRadians(float newOrientationRadians)
{
	float oldLen = GetLength();

	x = oldLen * cosf(newOrientationRadians);
	y = oldLen * sinf(newOrientationRadians);
}

void Vec2::SetOrientationDegrees(float newOrientationDegrees)
{
	float oldLen = GetLength();

	float newOrientationRadians = ConvertDegreesToRadians(newOrientationDegrees);

	x = oldLen * cosf(newOrientationRadians);
	y = oldLen * sinf(newOrientationRadians);
}

void Vec2::SetPolarRadians(float newOrientationRadians, float newLength)
{
	x = newLength * cosf(newOrientationRadians);
	y = newLength * sinf(newOrientationRadians);
}

void Vec2::SetPolarDegrees(float newOrientationDegrees, float newLength)
{
	float newOrientationRadians = ConvertDegreesToRadians(newOrientationDegrees);

	x = newLength * cosf(newOrientationRadians);
	y = newLength * sinf(newOrientationRadians);
}

void Vec2::Rotate90Degrees()
{
	float oldX = x;
	x = -y;
	y = oldX;
}

void Vec2::RotateMinus90Degrees()
{
	float oldX = x;
	x = y;
	y = -oldX;
}

void Vec2::RotateRadians(float deltaRadians)
{
	float oldLen = GetLength();

	float oldRadians = atan2f(y, x);
	float newRadians = oldRadians + deltaRadians;

	x = oldLen * cosf(newRadians);
	y = oldLen * sinf(newRadians);
}

void Vec2::RotateDegrees(float deltaDegrees)
{
	float oldLen = GetLength();

	float oldDegrees = Atan2Degrees(y, x);
	float newRadians = ConvertDegreesToRadians(oldDegrees + deltaDegrees);

	x = oldLen * cosf(newRadians);
	y = oldLen * sinf(newRadians);
}

void Vec2::SetLength(float newLength)
{
	Normalize();

	x *= newLength;
	y *= newLength;
}

void Vec2::ClampLength(float maxLength)
{
	float oldLen = GetLength();

	if (oldLen > maxLength)
	{
		SetLength(maxLength);
	}
}


void Vec2::Normalize()
{
	float oldLen = GetLength();

	float uniformScale = 1.f / oldLen;

	x *= uniformScale;
	y *= uniformScale;
}

float Vec2::NormalizeAndGetPreviousLength()
{
	float oldLen = GetLength();

	float uniformScale = 1.f / oldLen;

	x *= uniformScale;
	y *= uniformScale;
	
	return oldLen;
}

Vec2 const Vec2::GetReflected(Vec2 const& normalOfSurfaceToReflectOffOf) const
{
	float dotProduct = DotProduct2D(*this, normalOfSurfaceToReflectOffOf);
	Vec2 reflect = *this - (2 * dotProduct) * normalOfSurfaceToReflectOffOf;
	return reflect;
}

void Vec2::Reflect(Vec2 const& normalOfSurfaceToReflectOffOf)
{
	float dotProduct = DotProduct2D(*this, normalOfSurfaceToReflectOffOf);
	*this -= (2 * dotProduct) * normalOfSurfaceToReflectOffOf;

}

void Vec2::SetFromText(char const* text)
{
	if (!text) return;

	Strings parts = SplitStringOnDelimiter(text, ',');

	if (parts.size() != 2)
	{
		ERROR_AND_DIE(Stringf("Invalid Vec2 Text"));
	}

	x = static_cast<float>(atof(parts[0].c_str()));
	y = static_cast<float>(atof(parts[1].c_str()));
}


//-----------------------------------------------------------------------------------------------
Vec2 const Vec2::operator + ( Vec2 const& vecToAdd ) const
{
	return Vec2( x + vecToAdd.x, y + vecToAdd.y );
}


//-----------------------------------------------------------------------------------------------
Vec2 const Vec2::operator-( Vec2 const& vecToSubtract ) const
{
	return Vec2(x - vecToSubtract.x, y - vecToSubtract.y);
}


//------------------------------------------------------------------------------------------------
Vec2 const Vec2::operator-() const
{
	return Vec2( -x, -y );
}


//-----------------------------------------------------------------------------------------------
Vec2 const Vec2::operator*( float uniformScale ) const
{
	return Vec2(x * uniformScale, y * uniformScale);
}


//------------------------------------------------------------------------------------------------
Vec2 const Vec2::operator*( Vec2 const& vecToMultiply ) const
{
	return Vec2( x * vecToMultiply.x, y * vecToMultiply.y);
}


//-----------------------------------------------------------------------------------------------
Vec2 const Vec2::operator/( float inverseScale ) const
{
	return Vec2(x / inverseScale, y / inverseScale);
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator+=( Vec2 const& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator-=( Vec2 const& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( const float uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator=( Vec2 const& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
Vec2 const operator*( float uniformScale, Vec2 const& vecToScale )
{
	return Vec2( vecToScale.x * uniformScale, vecToScale.y * uniformScale );
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator==( Vec2 const& compare ) const
{
	return (x == compare.x && y == compare.y);
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=( Vec2 const& compare ) const
{
	return (x != compare.x || y != compare.y);
}

