#pragma once
struct Rgba8;

struct Vec4 
{

public:
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;

public:			
	Vec4() = default;
	explicit Vec4(float initialX, float initialY, float initialZ, float initialW);		

	//Accessors (const methods)
	float GetLength() const;
	float GetLengthXY() const;
	float GetLengthSquared() const;
	float GetLengthXYSquared() const;
	Vec4 const GetClamped(float maxLength) const;
	Vec4 const GetNormalized() const;
	Vec4 const GetClamped(float min, float max) const;

	Rgba8 ToRgba();

	// Operators (const)
	bool		operator==(Vec4 const& compare) const;
	bool		operator!=(Vec4 const& compare) const;
	Vec4 const	operator+(Vec4 const& vecToAdd) const;
	Vec4 const	operator-(Vec4 const& vecToSubtract) const;
	Vec4 const	operator-() const;
	Vec4 const	operator*(float uniformScale) const;
	Vec4 const	operator*(Vec4 const& vecToMultiply) const;
	Vec4 const	operator/(float inverseScale) const;

	// Operators (self-mutating / non-const)
	void		operator+=(Vec4 const& vecToAdd);
	void		operator-=(Vec4 const& vecToSubtract);
	void		operator*=(const float uniformScale);			
	void		operator/=(const float uniformDivisor);		
	void		operator=(Vec4 const& copyFrom);

	

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec2::
	friend Vec4 const operator*(float uniformScale, Vec4 const& vecToScale);
};