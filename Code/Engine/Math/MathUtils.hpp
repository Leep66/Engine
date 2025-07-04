#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

struct Vec2;
struct Vec3;
struct Vec4;
struct IntVec2;
struct AABB2;
struct Capsule2;
struct Disc2;
struct LineSegment2;
struct OBB2;
struct Triangle2;
struct Mat44;
class FloatRange;
struct AABB3;

struct RaycastResult2D
{
	bool m_didImpact = false;
	float m_impactDist = 0.f;
	Vec2 m_impactPos;
	Vec2 m_impactNormal;

	Vec2 m_rayFwdNormal;
	Vec2 m_rayStartPos;
	float m_rayMaxLength = 1.f;
};

struct Ray2
{
	Vec2 m_startPos;
	Vec2 m_fwdNormal;
	float m_maxLength = 1.f;
	Ray2() = default;
	Ray2(Vec2 start, Vec2 normal, float maxLength)
	{
		m_startPos = start;
		m_fwdNormal = normal;
		m_maxLength = maxLength;
	}
};

struct RaycastResult3D
{
	bool m_didImpact = false;
	float m_impactDist = 0.f;
	Vec3 m_impactPos = Vec3::ZERO;
	Vec3 m_impactNormal = Vec3::ZERO;

	Vec3 m_rayStart = Vec3::ZERO;
	Vec3 m_rayForwardNormal = Vec3::ZERO;
	float m_rayMaxLength = 1.f;
};

struct Ray3
{
	Vec3 m_rayStart = Vec3::ZERO;
	Vec3 m_rayForwardNormal = Vec3::ZERO;
	float m_rayMaxLength = 1.f;
	Ray3() = default;

	Ray3(Vec3 start, Vec3 normal, float maxLength)
	{
		m_rayStart = start;
		m_rayForwardNormal = normal;
		m_rayMaxLength = maxLength;
	}
};

enum class BillboardType
{
	NONE = -1,
	WORLD_UP_FACING,
	WORLD_UP_OPPOSING,
	FULL_FACING,
	FULL_OPPOSING,
	COUNT
};

float ConvertDegreesToRadians(float degrees);
float ConvertRadiansToDegrees(float radians);
float CosDegrees(float degrees);
float SinDegrees(float degrees);
float Atan2Degrees(float y, float x);
float GetShortestAngularDispDegrees(float startDegrees, float endDegrees);
float GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees);


float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB);
float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB);
float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB);



void TransformPosition2D(Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation);

void TransformPositionXY3D(Vec3& positionToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY);


float GetClamped(float value, float minValue, float maxValue);
float GetClampedZeroToOne(float value);
float Interpolate(float start, float end, float fractionTowardEnd);

Vec2 Interpolate(const Vec2& a, const Vec2& b, float t);
float GetFractionWithinRange(float value, float rangeStart, float rangeEnd);
float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd);
float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd);
int RoundDownToInt(float value);


float DotProduct2D(Vec2 const& a, Vec2 const& b);
float DotProduct3D(Vec3 const& a, Vec3 const& b);
float DotProduct4D(Vec4 const& a, Vec4 const& b);
float CrossProduct2D(Vec2 const& a, Vec2 const& b);
Vec3 CrossProduct3D(Vec3 const& a, Vec3 const& b);


bool DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB);
bool DoDiscAndAABBOverlap2D(Vec2 const& center, float radiusA, AABB2 bounds);
bool DoDiscAndOBBOverlap2D(Vec2 const& discCenter, float radius, OBB2 obb);
bool DoDiscAndCapsuleOverlap2D(Vec2 const& discCenter, float radius, Capsule2 capsule);

bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB);

bool DoAABBsOverlap3D(AABB3 const& first, AABB3 const& second);
bool DoSpheresOverlap3D(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB);
bool DoZCylindersOverlap3D(Vec2 cylinder1CenterXY, float cylinder1Radius, FloatRange cylinder1MinmaxZ, Vec2 cylinder2CenterXY, float cylinder2Radius, FloatRange cylinder2MinMaxZ);
bool DoSphereAndAABBOverlap3D(Vec3 sphereCenter, float sphereRadius, AABB3 box);
bool DoZCylinderAndAABBOverlap3D(Vec2 cylinderCenterXY, float cylinderRadius, FloatRange cylinderMinMaxZ, AABB3 box);
bool DoZCylinderAndSphereOverlap3D(Vec2 cylinderCenterXY, float cylinderRadius, FloatRange cylinderMinMaxZ, Vec3 sphereCenter, float sphereRadisu);
bool DoOBBsOverlap3D(Vec3 const& iA, Vec3 const& jA, Vec3 const& kA, Vec3 const& halfDimensionsA, Vec3 const& centerA,
	Vec3 const& iB, Vec3 const& jB, Vec3 const& kB, Vec3 const& halfDimensionsB, Vec3 const& centerB);
bool DoOBBAndSphereOverlap3D(Vec3 const& obbI, Vec3 const& obbJ, Vec3 const& obbK, Vec3 const& obbHalfDimensions, Vec3 const& obbCenter,
	Vec3 const& sphereCenter, float sphereRadius);
bool DoOBBAndPlaneOverlap3D(Vec3 const& obbI, Vec3 const& obbJ, Vec3 const& obbK, Vec3 const& obbHalfDimensions, Vec3 const& obbCenter,
	Vec3 const& planeNormal, float planeDistFromOrigin);
bool DoPlaneAndSphereOverlap3D(Vec3 const& planeNormal, float planeDistFromOrigin, Vec3 const& sphereCenter, float sphereRadius);
bool DoPlaneAndAABB3Overlap3D(Vec3 const& planeNormal, float planeDistFromOrigin, AABB3 const& aabb);


float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b);


int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB);
float GetProjectedLength2D(Vec2 const& vertorToProject, Vec2 const& vertorToProjectOnto);
Vec2 const GetProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& vertorToProjectOnto);


void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation);
void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation);


bool PushDiscOutOfPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint);
bool PushDiscOutOfDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius);
bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius);
bool PushDiscOutOfAABB2D(Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox);

void BounceDiscOutOfDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2& mobileDiscVelocity, float mobileDiscElasticity, 
	Vec2 const& fixedDiscCenter, float fixedDiscRadius, float fixedDiscElasticity);
void BounceDiscOutOfOBB2D(Vec2& discCenter, float discRadius, Vec2& discVelocity, float discElasticity,
	Vec2 const& obbCenter, Vec2 const& obbIBasis, Vec2 const& obbHalfDimensions, float obbElasticity);
void BounceDiscOutOfCapsule2D(Vec2& discCenter, float discRadius, Vec2& discVelocity, float discElasticity,
	Vec2 const& capsuleStart, Vec2 const& capsuleEnd, float capsuleRadius, float capsuleElasticity);
void BounceDiscOutOfLineSegment2D(Vec2& discCenter, float discRadius, Vec2& discVelocity, float discElasticity,
	Vec2 const& lineStart, Vec2 const& lineEnd, float lineElasticity);
void BounceDiscsOutOfEachOther(Vec2& discCenterA, float discRadiusA, Vec2& discVelocityA, float discElasticityA,
	Vec2& discCenterB, float discRadiusB, Vec2& discVelocityB, float discElasticityB);
bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius);
bool IsPointInsideDisc2D(Vec2 const& point, Disc2 disc);
bool IsPointInsideSphere3D(Vec3 const& point, Vec3 sphereCenter, float sphereRadius);

bool IsPointInsideCylinderZ3D(Vec3 const& point, Vec2 const& cylinCenterXY, FloatRange const& cylinMinMaxZ, float cylinRadius);

bool IsPointInsideTriangle2D(Vec2 const& point, Vec2 const& triCCW0, Vec2 const& triCCW1, Vec2 const& triCCW2);
bool IsPointInsideTriangle2D(Vec2 const& point, Triangle2 triangle);

bool IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box);
bool IsPointInsideAABB3D(Vec3 const& point, AABB3 const& box);
bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& box);

bool IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius);
bool IsPointInsideCapsule2D(Vec2 const& point, Capsule2 capsule);

bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius);
bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius);

bool IsPointInsideOBB3D(Vec3 const& point, Vec3 const& i, Vec3 const& j, Vec3 const& k, Vec3 const& halfDimensions, Vec3 const& center);
bool IsPointInFrontOfPlane3D(Vec3 const& point, Vec3 const& normal, float distFromOrigin);

Vec2 GetNearestPointOnDisc2D(Vec2 const& referencePos, Vec2 const& discCenter, float discRadius);
Vec2 GetNearestPointOnDisc2D(Vec2 const& referencePos, Disc2 disc);

Vec2 GetNearestPointOnAABB2D(Vec2 const& referencePos, AABB2 const& alignedBox);

Vec2 GetNearestPointOnOBB2D(Vec2 const& referencePos, OBB2 const& orientedBox);

Vec2 GetNearestPointOnInfiniteLine2D(Vec2 const& referencePos, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine);

Vec2 GetNearestPointOnLineSegment2D(Vec2 const& referencePos, Vec2 const& start, Vec2 const& end);
Vec2 GetNearestPointOnLineSegment2D(Vec2 const& referencePos, LineSegment2 lineSegment);

Vec2 GetNearestPointOnCapsule2D(Vec2 const& referencePos, Vec2 const& boneStart, Vec2 const& boneEnd, float radius);
Vec2 GetNearestPointOnCapsule2D(Vec2 const& referencePos, Capsule2 capsule);

Vec2 GetNearestPointOnTriangle2D(Vec2 const& referencePos, Vec2 const& triCCW0, Vec2 const& triCCW1, Vec2 const& triCCW2);
Vec2 GetNearestPointOnTriangle2D(Vec2 const& referencePos, Triangle2 triangle);

Vec3 GetNearestPointOnAABB3D(Vec3 const& referencePos, Vec3 const& mins, Vec3 const& maxs);
Vec3 GetNearestPointOnSphere3D(Vec3 const& referencePos, Vec3 const& center, float radius);
Vec3 GetNearestPointOnZCylinder3D(Vec3 const& referencePos, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius);

Vec3 GetNearestPointOnOBB3D(Vec3 const& referencePos, Vec3 const& i, Vec3 const& j, Vec3 const& k, Vec3 const& halfDimensions, Vec3 const& center);
Vec3 GetNearestPointOnPlane3D(Vec3 const& referencePos, Vec3 const& normal, float distFromOrigin);

Vec3 RotateVectorInPlane(Vec3 vec, float angleDegrees);
RaycastResult2D RaycastVsDisc2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 discCenter, float discRadius);
RaycastResult2D RaycastVsDisc2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Disc2 disc);

RaycastResult2D RaycastVsLineSegment2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 lineStart, Vec2 lineEnd);
RaycastResult2D RaycastVsLineSegment2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, LineSegment2 lineSegment);


RaycastResult2D RaycastVsAABB2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 mins, Vec2 maxs);
RaycastResult2D RaycastVsAABB2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, AABB2 bound);


float NormalizeByte(unsigned char b);
unsigned char DenormalizeByte(float zeroToOne);

Mat44 GetBillboardMatrix(BillboardType billboardType, Mat44 const& targteMatrix, const Vec3& billboardPosition,
	const Vec2& billboardScale = Vec2(1.f, 1.f));

RaycastResult3D RaycastVsAABB3D(Vec3 rayStart, Vec3 rayForwardNormal, float rayLength, AABB3 box);
RaycastResult3D RaycastVsSphere3D(Vec3 rayStart, Vec3 rayForwardNormal, float rayLength, Vec3 sphereCenter, float sphereRadius);
RaycastResult3D RaycastVsCylinderZ3D(Vec3 rayStart, Vec3 rayForwardNormal, float rayLength, Vec2 const& centerXY, FloatRange const& minMaxZ, float radiusXY);
RaycastResult3D RaycastVsOBB3D(Vec3 rayStart, Vec3 rayForwardNormal, float rayLength, Vec3 const& obbI, Vec3 const& obbJ, Vec3 const& obbK, Vec3 const& obbHalfDimensions, Vec3 const& obbCenter);
RaycastResult3D RaycastVsPlane3D(Vec3 rayStart, Vec3 rayForwardNormal, float rayLength, Vec3 const& planeNormal, float planeDistFromOrigin);
float ComputeCubicBezier1D(float A, float B, float C, float D, float t);
float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float t);