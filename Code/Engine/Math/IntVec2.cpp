#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "math.h"

const IntVec2 IntVec2::ZERO(0, 0);

IntVec2::IntVec2(const IntVec2& copyFrom)
	:x(copyFrom.x)
	,y(copyFrom.y)
{
}

IntVec2::IntVec2(int initialX, int initialY)
	:x(initialX)
	,y(initialY)
{
}

float IntVec2::GetLength() const
{
	return sqrtf(static_cast<float>(x) * static_cast<float>(x) +
		static_cast<float>(y) * static_cast<float>(y));
}

int IntVec2::GetTaxicabLength() const
{
	return (abs(x) + abs(y));
}

int IntVec2::GetLengthSquared() const
{
	return (x * x + y * y);
}

float IntVec2::GetOrientationRadians() const
{
	return atan2f(static_cast<float>(y), static_cast<float>(x));
}

float IntVec2::GetOrientationDegrees() const
{
	return Atan2Degrees(static_cast<float>(y), static_cast<float>(x));
}

IntVec2 const IntVec2::GetRotated90Degrees() const
{
	return IntVec2(-y, x);
}

IntVec2 const IntVec2::GetRotatedMinus90Degrees() const
{
	return IntVec2(y, -x);
}

Vec2 IntVec2::ToVec2() const
{
	return Vec2((float)x, (float)y);
}

void IntVec2::Rotate90Degrees()
{
	int oldX = x;
	x = -y;
	y = oldX;
}

void IntVec2::RotateMinus90Degrees()
{
	int oldX = x;
	x = y;
	y = -oldX;
}

void IntVec2::SetFromText(char const* text)
{
	if (!text) return;

	Strings parts = SplitStringOnDelimiter(text, ',');
	if (parts.size() != 2)
	{
		ERROR_AND_DIE(Stringf("Invalid IntVec2 Text"));
	}

	x = atoi(parts[0].c_str());
	y = atoi(parts[1].c_str());
}

void IntVec2::operator=(const IntVec2& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
}

IntVec2 const IntVec2::operator+(IntVec2 const& vecToAdd) const
{
	IntVec2 result;
	result.x = x + vecToAdd.x;
	result.y = y + vecToAdd.y;
	return result;
}

IntVec2 const IntVec2::operator-(IntVec2 const& vecToSubtract) const
{
	IntVec2 result;
	result.x = x - vecToSubtract.x;
	result.y = y - vecToSubtract.y;
	return result;
}

bool IntVec2::operator==(IntVec2 const& vecToCompare) const
{
	return x == vecToCompare.x && y == vecToCompare.y;
}

bool IntVec2::operator!=(IntVec2 const& vecToCompare) const
{
	return x != vecToCompare.x || y != vecToCompare.y;
}

void IntVec2::operator+=(IntVec2 const& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}