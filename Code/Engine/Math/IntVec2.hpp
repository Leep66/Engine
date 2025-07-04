#pragma once
#include "Engine/Math/Vec2.hpp"

struct IntVec2
{

public:
	int x = 0;
	int y = 0;

public:
	~IntVec2() {}
	IntVec2() {}
	IntVec2(const IntVec2& copyFrom);
	explicit IntVec2(int initialX, int initialY);

	static const IntVec2 ZERO;

	float GetLength() const;
	int GetTaxicabLength() const;
	int GetLengthSquared() const;
	float GetOrientationRadians() const;
	float GetOrientationDegrees() const;
	IntVec2 const GetRotated90Degrees() const;
	IntVec2 const GetRotatedMinus90Degrees() const;
	Vec2 ToVec2() const;

	void Rotate90Degrees();
	void RotateMinus90Degrees();

	void SetFromText(char const* text);

	void operator=(const IntVec2& copyFrom);
	IntVec2 const operator+(IntVec2 const& vecToAdd) const;
	IntVec2 const operator-(IntVec2 const& vecToSubtract) const;
	bool operator==(IntVec2 const& vecToCompare) const;
	bool operator!=(IntVec2 const& vecToCompare) const;
	void operator+=(IntVec2 const& vecToAdd);
};