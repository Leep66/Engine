#pragma once
#include <string>
#include "Engine/Math/Vec3.hpp"

struct IntVec3
{
public:
	int x = 0;
	int y = 0;
	int z = 0;

public:
	~IntVec3() {}
	IntVec3() {}
	IntVec3(const IntVec3& copyFrom);
	explicit IntVec3(int initialX, int initialY, int initialZ);

	static const IntVec3 ZERO;

	float GetLength() const;
	int   GetTaxicabLength() const;
	int   GetLengthSquared() const;
	Vec3  ToVec3() const;

	void  SetFromText(char const* text);

	void        operator=(const IntVec3& copyFrom);
	IntVec3     operator+(IntVec3 const& vecToAdd) const;
	IntVec3     operator-(IntVec3 const& vecToSubtract) const;
	IntVec3     operator*(int scale) const;
	bool        operator==(IntVec3 const& vecToCompare) const;
	bool        operator!=(IntVec3 const& vecToCompare) const;
	void        operator+=(IntVec3 const& vecToAdd);
};
