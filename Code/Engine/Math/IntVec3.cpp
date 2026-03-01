#include "Engine/Math/IntVec3.hpp"
#include <cmath>        // sqrtf
#include <cstdlib>      // atoi
#include <cstring>      // strlen, strchr

const IntVec3 IntVec3::ZERO = IntVec3(0, 0, 0);

IntVec3::IntVec3(const IntVec3& copyFrom)
	: x(copyFrom.x), y(copyFrom.y), z(copyFrom.z)
{
}

IntVec3::IntVec3(int initialX, int initialY, int initialZ)
	: x(initialX), y(initialY), z(initialZ)
{
}

float IntVec3::GetLength() const
{
	return std::sqrt((float)(x * x + y * y + z * z));
}

int IntVec3::GetTaxicabLength() const
{
	// |x| + |y| + |z|
	const int ax = (x >= 0) ? x : -x;
	const int ay = (y >= 0) ? y : -y;
	const int az = (z >= 0) ? z : -z;
	return ax + ay + az;
}

int IntVec3::GetLengthSquared() const
{
	return x * x + y * y + z * z;
}

Vec3 IntVec3::ToVec3() const
{
	return Vec3((float)x, (float)y, (float)z);
}

void IntVec3::SetFromText(char const* text)
{
	if (text == nullptr || *text == '\0') {
		x = y = z = 0;
		return;
	}

	char const* c1 = std::strchr(text, ',');
	if (!c1) { x = y = z = 0; return; }
	char const* c2 = std::strchr(c1 + 1, ',');
	if (!c2) { x = y = z = 0; return; }

	x = std::atoi(std::string(text, c1 - text).c_str());
	y = std::atoi(std::string(c1 + 1, c2 - (c1 + 1)).c_str());
	z = std::atoi(std::string(c2 + 1).c_str());
}

void IntVec3::operator=(const IntVec3& copyFrom)
{
	x = copyFrom.x; y = copyFrom.y; z = copyFrom.z;
}

IntVec3 IntVec3::operator+(IntVec3 const& vecToAdd) const
{
	return IntVec3(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z);
}

IntVec3 IntVec3::operator-(IntVec3 const& vecToSubtract) const
{
	return IntVec3(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z);
}

IntVec3 IntVec3::operator*(int scale) const
{
	return IntVec3(x * scale, y * scale, z * scale);
}

bool IntVec3::operator==(IntVec3 const& vecToCompare) const
{
	return (x == vecToCompare.x) && (y == vecToCompare.y) && (z == vecToCompare.z);
}

bool IntVec3::operator!=(IntVec3 const& vecToCompare) const
{
	return !(*this == vecToCompare);
}

void IntVec3::operator+=(IntVec3 const& vecToAdd)
{
	x += vecToAdd.x; y += vecToAdd.y; z += vecToAdd.z;
}
