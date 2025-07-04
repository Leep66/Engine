#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"

struct EulerAngles;

struct Vec3 
{

public:
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

	static const Vec3 ZERO;
	static const Vec3 ONE;


public:			
	Vec3() = default;
	explicit Vec3(float initialX, float initialY, float initialZ);		
	explicit Vec3(Vec2 xy, float z);

	//Accessors (const methods)
	float GetLength() const;
	float GetLengthXY() const;
	Vec2 GetXY() const;
	
	Vec3 IgnoreZ() const;
	float GetLengthSquared() const;
	float GetLengthXYSquared() const;
	float GetAngleAboutZRadians() const;
	float GetAngleAboutZDegrees() const;
	Vec3 const GetRotatedAboutZRadians(float deltaRadians) const;
	Vec3 const GetRotatedAboutZDegrees(float deltaDegrees) const;
	Vec3 const GetClamped(float maxLength) const;
	Vec3 const GetClamped(float min, float max) const;
	Vec3 const GetNormalized() const;

	EulerAngles GetAsEulerAngles() const;

	Rgba8 ToRgba();


	static Vec3 const MakeFromPolarDegrees(float orientationDegrees, float zDegrees, float length = 1.f);



	// Operators (const)
	bool		operator==(Vec3 const& compare) const;		
	bool		operator!=(Vec3 const& compare) const;		
	Vec3 const	operator+(Vec3 const& vecToAdd) const;		
	Vec3 const	operator-(Vec3 const& vecToSubtract) const;	
	Vec3 const	operator-() const;								
	Vec3 const	operator*(float uniformScale) const;			
	Vec3 const	operator*(Vec3 const& vecToMultiply) const;	
	Vec3 const	operator/(float inverseScale) const;			

	// Operators (self-mutating / non-const)
	void		operator+=(Vec3 const& vecToAdd);				
	void		operator-=(Vec3 const& vecToSubtract);		
	void		operator*=(const float uniformScale);			
	void		operator/=(const float uniformDivisor);		
	void		operator=(Vec3 const& copyFrom);

	

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec2::
	friend Vec3 const operator*(float uniformScale, Vec3 const& vecToScale);	
};