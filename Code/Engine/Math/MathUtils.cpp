#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Disc2.hpp"
#include "Engine/Math/Triangle2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/FloatRange.hpp"
#include <cmath>
#include <algorithm>

constexpr float PI = 3.1415926535897932384626433832795f;
float ConvertDegreesToRadians(float degrees)
{
	return degrees * (PI / 180.0f);
}

float ConvertRadiansToDegrees(float radians)
{
	return radians * (180.0f / PI);
}

float CosDegrees(float degrees)
{
	float radians = ConvertDegreesToRadians(degrees);
	return cosf(radians);
}

float SinDegrees(float degrees)
{
	float radians = ConvertDegreesToRadians(degrees);
	return sinf(radians);
}

float Atan2Degrees(float y, float x)
{
	float radians = atan2f(y, x);
	return ConvertRadiansToDegrees(radians);
}

float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB)
{
	float deltaX = positionB.x - positionA.x;
	float deltaY = positionB.y - positionA.y;
	return sqrtf((deltaX * deltaX) + (deltaY * deltaY));
}

float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB)
{
	float deltaX = positionB.x - positionA.x;
	float deltaY = positionB.y - positionA.y;
	return ((deltaX * deltaX) + (deltaY * deltaY));
}

float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float deltaX = positionB.x - positionA.x;
	float deltaY = positionB.y - positionA.y;
	float deltaZ = positionB.z - positionA.z;

	return sqrtf((deltaX * deltaX) + (deltaY * deltaY) + (deltaZ * deltaZ));
}

float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float deltaX = positionB.x - positionA.x;
	float deltaY = positionB.y - positionA.y;
	float deltaZ = positionB.z - positionA.z;

	return ((deltaX * deltaX) + (deltaY * deltaY) + (deltaZ * deltaZ));
}

float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float deltaX = positionB.x - positionA.x;
	float deltaY = positionB.y - positionA.y;

	return sqrtf((deltaX * deltaX) + (deltaY * deltaY));
}

float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float deltaX = positionB.x - positionA.x;
	float deltaY = positionB.y - positionA.y;

	return ((deltaX * deltaX) + (deltaY * deltaY));
}

bool DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB)
{
	float centerDistAtoB = GetDistance2D(centerA, centerB);
	float sumRadius = radiusA + radiusB;
	return (centerDistAtoB <= sumRadius);
}

bool DoDiscAndAABBOverlap2D(Vec2 const& center, float radius, AABB2 bounds)
{
	Vec2 closestPoint = GetNearestPointOnAABB2D(center, bounds);
	float distSquared = (closestPoint - center).GetLengthSquared();

	return distSquared <= radius * radius;
}

bool DoDiscAndOBBOverlap2D(Vec2 const& discCenter, float radius, OBB2 obb)
{
	Vec2 closestPoint = GetNearestPointOnOBB2D(discCenter, obb);

	Vec2 displacement = discCenter - closestPoint;
	float distanceSquared = displacement.GetLengthSquared();

	return distanceSquared <= (radius * radius);
}

bool DoDiscAndCapsuleOverlap2D(Vec2 const& discCenter, float radius, Capsule2 capsule)
{
	Vec2 closestPoint = GetNearestPointOnCapsule2D(discCenter, capsule);

	float distToLineSegment = (discCenter - closestPoint).GetLength();
	return distToLineSegment <= radius;
}

bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB)
{
	float centerDistAtoB = GetDistance3D(centerA, centerB);
	float sumRadius = radiusA + radiusB;
	return (centerDistAtoB <= sumRadius);
}

bool DoAABBsOverlap3D(AABB3 const& first, AABB3 const& second)
{
	bool overlapX = (first.m_mins.x <= second.m_maxs.x) && (first.m_maxs.x >= second.m_mins.x);
	bool overlapY = (first.m_mins.y <= second.m_maxs.y) && (first.m_maxs.y >= second.m_mins.y);
	bool overlapZ = (first.m_mins.z <= second.m_maxs.z) && (first.m_maxs.z >= second.m_mins.z);

	return overlapX && overlapY && overlapZ;
}

bool DoSpheresOverlap3D(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB)
{
	Vec3 delta = centerB - centerA;
	float distSquared = delta.GetLengthSquared();
	float radiusSum = radiusA + radiusB;
	float radiusSumSquared = radiusSum * radiusSum;

	return distSquared < radiusSumSquared;
}

bool DoZCylindersOverlap3D(Vec2 cylinder1CenterXY, float cylinder1Radius, FloatRange cylinder1MinMaxZ,
	Vec2 cylinder2CenterXY, float cylinder2Radius, FloatRange cylinder2MinMaxZ)
{
	bool overlapZ = (cylinder1MinMaxZ.m_min <= cylinder2MinMaxZ.m_max) &&
		(cylinder1MinMaxZ.m_max >= cylinder2MinMaxZ.m_min);

	if (!overlapZ)
	{
		return false;
	}

	Vec2 deltaXY = cylinder2CenterXY - cylinder1CenterXY;
	float distSquaredXY = deltaXY.GetLengthSquared();
	float radiusSum = cylinder1Radius + cylinder2Radius;
	float radiusSumSquared = radiusSum * radiusSum;

	bool overlapXY = distSquaredXY < radiusSumSquared;

	return overlapXY;
}

bool DoSphereAndAABBOverlap3D(Vec3 sphereCenter, float sphereRadius, AABB3 box)
{
	Vec3 nearestPoint = GetNearestPointOnAABB3D(sphereCenter, box.m_mins, box.m_maxs);

	Vec3 delta = sphereCenter - nearestPoint;
	float distSquared = delta.GetLengthSquared();

	float radiusSquared = sphereRadius * sphereRadius;
	return distSquared < radiusSquared;
}

bool DoZCylinderAndAABBOverlap3D(Vec2 cylinderCenterXY, float cylinderRadius, FloatRange cylinderMinMaxZ, AABB3 box)
{
	bool overlapZ = (cylinderMinMaxZ.m_min <= box.m_maxs.z) &&
		(cylinderMinMaxZ.m_max >= box.m_mins.z);

	if (!overlapZ)
	{
		return false;
	}

	Vec2 boxCenterXY((box.m_mins.x + box.m_maxs.x) * 0.5f, (box.m_mins.y + box.m_maxs.y) * 0.5f);
	Vec2 boxHalfExtents((box.m_maxs.x - box.m_mins.x) * 0.5f, (box.m_maxs.y - box.m_mins.y) * 0.5f);

	Vec2 deltaXY = cylinderCenterXY - boxCenterXY;

	Vec2 clampedDeltaXY;
	clampedDeltaXY.x = (deltaXY.x < -boxHalfExtents.x) ? -boxHalfExtents.x : (deltaXY.x > boxHalfExtents.x ? boxHalfExtents.x : deltaXY.x);
	clampedDeltaXY.y = (deltaXY.y < -boxHalfExtents.y) ? -boxHalfExtents.y : (deltaXY.y > boxHalfExtents.y ? boxHalfExtents.y : deltaXY.y);

	Vec2 nearestPointXY = boxCenterXY + clampedDeltaXY;
	Vec2 deltaToNearest = cylinderCenterXY - nearestPointXY;
	float distSquaredXY = deltaToNearest.GetLengthSquared();

	float radiusSquared = cylinderRadius * cylinderRadius;
	bool overlapXY = distSquaredXY < radiusSquared;

	return overlapXY;
}

bool DoZCylinderAndSphereOverlap3D(Vec2 cylinderCenterXY, float cylinderRadius, FloatRange cylinderMinMaxZ, Vec3 sphereCenter, float m_sphereRadius)
{
	bool overlapZ = (cylinderMinMaxZ.m_min <= (sphereCenter.z + m_sphereRadius)) &&
		(cylinderMinMaxZ.m_max >= (sphereCenter.z - m_sphereRadius));

	if (!overlapZ)
	{
		return false;
	}

	Vec2 sphereCenterXY(sphereCenter.x, sphereCenter.y);
	Vec2 deltaXY = sphereCenterXY - cylinderCenterXY;
	float distSquaredXY = deltaXY.GetLengthSquared();

	float radiusSum = cylinderRadius + m_sphereRadius;
	float radiusSumSquared = radiusSum * radiusSum;

	bool overlapXY = distSquaredXY < radiusSumSquared;

	return overlapXY;
}

bool DoOBBsOverlap3D(Vec3 const& iA, Vec3 const& jA, Vec3 const& kA, Vec3 const& halfDimensionsA, Vec3 const& centerA,
	Vec3 const& iB, Vec3 const& jB, Vec3 const& kB, Vec3 const& halfDimensionsB, Vec3 const& centerB)
{
	Vec3 nearestOnB = GetNearestPointOnOBB3D(centerA, iB, jB, kB, halfDimensionsB, centerB);
	if (IsPointInsideOBB3D(nearestOnB, iA, jA, kA, halfDimensionsA, centerA)) {
		return true;
	}

	Vec3 nearestOnA = GetNearestPointOnOBB3D(centerB, iA, jA, kA, halfDimensionsA, centerA);
	if (IsPointInsideOBB3D(nearestOnA, iB, jB, kB, halfDimensionsB, centerB)) {
		return true;
	}

	return false;
}

bool DoOBBAndSphereOverlap3D(Vec3 const& obbI, Vec3 const& obbJ, Vec3 const& obbK, Vec3 const& obbHalfDimensions, Vec3 const& obbCenter, Vec3 const& sphereCenter, float sphereRadius)
{
	Vec3 nearestPoint = GetNearestPointOnOBB3D(sphereCenter, obbI, obbJ, obbK, obbHalfDimensions, obbCenter);
	Vec3 displacement = sphereCenter - nearestPoint;
	float distSq = displacement.GetLengthSquared();

	return distSq <= sphereRadius * sphereRadius;
}
bool DoPlaneAndSphereOverlap3D(Vec3 const& planeNormal, float planeDistFromOrigin, Vec3 const& sphereCenter, float sphereRadius)
{
	float distance = DotProduct3D(sphereCenter, planeNormal) - planeDistFromOrigin;
	float distSquared = distance * distance;

	return distSquared <= sphereRadius * sphereRadius;
}

bool DoPlaneAndAABB3Overlap3D(Vec3 const& planeNormal, float planeDistFromOrigin, AABB3 const& aabb)
{
	Vec3 corners[8];

	corners[0] = Vec3(aabb.m_mins.x, aabb.m_mins.y, aabb.m_maxs.z);
	corners[1] = Vec3(aabb.m_maxs.x, aabb.m_mins.y, aabb.m_maxs.z);
	corners[2] = Vec3(aabb.m_maxs.x, aabb.m_maxs.y, aabb.m_maxs.z);
	corners[3] = Vec3(aabb.m_mins.x, aabb.m_maxs.y, aabb.m_maxs.z);

	corners[4] = Vec3(aabb.m_mins.x, aabb.m_mins.y, aabb.m_mins.z);
	corners[5] = Vec3(aabb.m_maxs.x, aabb.m_mins.y, aabb.m_mins.z);
	corners[6] = Vec3(aabb.m_maxs.x, aabb.m_maxs.y, aabb.m_mins.z);
	corners[7] = Vec3(aabb.m_mins.x, aabb.m_maxs.y, aabb.m_mins.z);

	bool anyPointInFront = false;
	bool anyPointBehind = false;

	for (int i = 0; i < 8; ++i)
	{
		bool isInFront = IsPointInFrontOfPlane3D(corners[i], planeNormal, planeDistFromOrigin);

		if (isInFront)
		{
			anyPointInFront = true;
		}
		else
		{
			anyPointBehind = true;
		}
	}

	return anyPointInFront && anyPointBehind;
}




bool DoOBBAndPlaneOverlap3D(Vec3 const& obbI, Vec3 const& obbJ, Vec3 const& obbK, Vec3 const& obbHalfDimensions, Vec3 const& obbCenter, Vec3 const& planeNormal, float planeDistFromOrigin)
{
	if (IsPointInFrontOfPlane3D(obbCenter, planeNormal, planeDistFromOrigin))
	{
		return true;
	}

	Vec3 corners[8];
	corners[0] = obbCenter + obbI * obbHalfDimensions.x + obbJ * obbHalfDimensions.y + obbK * obbHalfDimensions.z;
	corners[1] = obbCenter - obbI * obbHalfDimensions.x + obbJ * obbHalfDimensions.y + obbK * obbHalfDimensions.z;
	corners[2] = obbCenter - obbI * obbHalfDimensions.x - obbJ * obbHalfDimensions.y + obbK * obbHalfDimensions.z;
	corners[3] = obbCenter + obbI * obbHalfDimensions.x - obbJ * obbHalfDimensions.y + obbK * obbHalfDimensions.z;
	corners[4] = obbCenter + obbI * obbHalfDimensions.x + obbJ * obbHalfDimensions.y - obbK * obbHalfDimensions.z;
	corners[5] = obbCenter - obbI * obbHalfDimensions.x + obbJ * obbHalfDimensions.y - obbK * obbHalfDimensions.z;
	corners[6] = obbCenter - obbI * obbHalfDimensions.x - obbJ * obbHalfDimensions.y - obbK * obbHalfDimensions.z;
	corners[7] = obbCenter + obbI * obbHalfDimensions.x - obbJ * obbHalfDimensions.y - obbK * obbHalfDimensions.z;

	for (int i = 0; i < 8; ++i) 
	{
		if (IsPointInFrontOfPlane3D(corners[i], planeNormal, planeDistFromOrigin)) 
		{
			return true;
		}

		
	}
	return false;
}


void TransformPosition2D(Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation)
{
	float curDegrees = Atan2Degrees(posToTransform.y, posToTransform.x);
	float oldLen = sqrtf(posToTransform.x * posToTransform.x + posToTransform.y * posToTransform.y);

	oldLen *= uniformScale;

	float newDegrees = curDegrees + rotationDegrees;

	float newRadians = ConvertDegreesToRadians(newDegrees);

	posToTransform.x = oldLen * cosf(newRadians);
	posToTransform.y = oldLen * sinf(newRadians);

	posToTransform.x += translation.x;
	posToTransform.y += translation.y;
}

void TransformPositionXY3D(Vec3& positionToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY)
{
	float curDegrees = Atan2Degrees(positionToTransform.y, positionToTransform.x);
	float oldLen = sqrtf(positionToTransform.x * positionToTransform.x + positionToTransform.y * positionToTransform.y);

	oldLen *= scaleXY;

	float newDegrees = curDegrees + zRotationDegrees;

	float newRadians = ConvertDegreesToRadians(newDegrees);

	positionToTransform.x = oldLen * cosf(newRadians);
	positionToTransform.y = oldLen * sinf(newRadians);

	positionToTransform.x += translationXY.x;
	positionToTransform.y += translationXY.y;
}

float GetClamped(float value, float minValue, float maxValue)
{
	float returnValue = value;
	if (value < minValue)
	{
		returnValue = minValue;
	}
	else if (value > maxValue)
	{
		returnValue = maxValue;
	}

	return returnValue;
}

float GetClampedZeroToOne(float value)
{
	float returnValue = value;
	if (value < 0)
	{
		returnValue = 0;
	}
	else if (value > 1)
	{
		returnValue = 1;
	}

	return returnValue;
}

float Interpolate(float start, float end, float fractionTowardEnd)
{
	return (start + (end - start) * fractionTowardEnd);
}

Vec2 Interpolate(const Vec2& a, const Vec2& b, float t)
{
	return a + (b - a) * t;
}

float GetFractionWithinRange(float value, float rangeStart, float rangeEnd)
{
	return ((value - rangeStart) / (rangeEnd - rangeStart));
}

float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float inRangeFraction = GetFractionWithinRange(inValue, inStart, inEnd);
	float outValue = Interpolate(outStart, outEnd, inRangeFraction);

	return outValue;
}

float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float inRangeFraction = GetFractionWithinRange(inValue, inStart, inEnd);
	float outValue = Interpolate(outStart, outEnd, inRangeFraction);

	if (outStart < outEnd) {
		if (outValue <= outStart) {
			outValue = outStart;
		}
		else if (outValue >= outEnd) {
			outValue = outEnd;
		}
	}
	else {
		if (outValue >= outStart) {
			outValue = outStart;
		}
		else if (outValue <= outEnd) {
			outValue = outEnd;
		}
	}

	return outValue;
}

int RoundDownToInt(float value)
{
	return static_cast<int>(floorf(value));
}

float GetShortestAngularDispDegrees(float startDegrees, float endDegrees)
{
	float disp = endDegrees - startDegrees;
	while (disp > 180.f)
	{
		disp -= 360.f;
	}
	while (disp < -180.f)
	{
		disp += 360.f;
	}
	return disp;
}

float GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees)
{
	while (currentDegrees >= 360.f) currentDegrees -= 360.f;
	while (currentDegrees < 0.f) currentDegrees += 360.f;

	while (goalDegrees >= 360.f) goalDegrees -= 360.f;
	while (goalDegrees < 0.f) goalDegrees += 360.f;

	float angleDifference = goalDegrees - currentDegrees;
	if (angleDifference > 180.f) {
		angleDifference -= 360.f;
	}
	if (angleDifference < -180.f) {
		angleDifference += 360.f;
	}

	if (angleDifference > maxDeltaDegrees) {
		angleDifference = maxDeltaDegrees;
	}
	if (angleDifference < -maxDeltaDegrees) {
		angleDifference = -maxDeltaDegrees;
	}

	float newDegrees = currentDegrees + angleDifference;
	while (newDegrees >= 360.f) newDegrees -= 360.f;
	while (newDegrees < 0.f) newDegrees += 360.f;

	return newDegrees;
}


float DotProduct2D(Vec2 const& a, Vec2 const& b)
{
	return (a.x * b.x) + (a.y * b.y);
}

float DotProduct3D(Vec3 const& a, Vec3 const& b)
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

float DotProduct4D(Vec4 const& a, Vec4 const& b)
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

float CrossProduct2D(Vec2 const& a, Vec2 const& b)
{
	return (a.x * b.y - a.y * b.x);
}

Vec3 CrossProduct3D(Vec3 const& a, Vec3 const& b)
{
	return Vec3(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	);
}


float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b)
{
	float cosOfAngle = DotProduct2D(a.GetNormalized(), b.GetNormalized());
	cosOfAngle = GetClamped(cosOfAngle, -1.f, 1.f);
	float angleRadians = acosf(cosOfAngle);
	return ConvertRadiansToDegrees(angleRadians);
}

int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB)
{
	return abs(pointA.x - pointB.x) + abs(pointA.y - pointB.y);
}

float GetProjectedLength2D(Vec2 const& vertorToProject, Vec2 const& vertorToProjectOnto)
{
	float dotProduct = DotProduct2D(vertorToProject, vertorToProjectOnto);
	float magnitude = vertorToProjectOnto.GetLength();
	float projectedLength = dotProduct / magnitude;
	return projectedLength;
}

Vec2 const GetProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& vertorToProjectOnto)
{
	float projectedLength = GetProjectedLength2D(vectorToProject, vertorToProjectOnto);
	Vec2 unitVector = vertorToProjectOnto.GetNormalized();
	Vec2 projectedOnto = projectedLength * unitVector;
	return projectedOnto;
}

void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation)
{
	Vec2 newPos;
	newPos.x = posToTransform.x * iBasis.x + posToTransform.y * jBasis.x;
	newPos.y = posToTransform.x * iBasis.y + posToTransform.y * jBasis.y;
	posToTransform = newPos + translation;
}

void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation)
{
	Vec2 newPos(posToTransform.x, posToTransform.y);
	TransformPosition2D(newPos, iBasis, jBasis, translation);
	posToTransform.x = newPos.x;
	posToTransform.y = newPos.y;
}

bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius)
{
	float distSquared = GetDistanceSquared2D(point, discCenter);
	return distSquared <= (discRadius * discRadius);

}

bool IsPointInsideDisc2D(Vec2 const& point, Disc2 disc)
{
	return IsPointInsideDisc2D(point, disc.m_center, disc.m_radius);
}

bool IsPointInsideSphere3D(Vec3 const& point, Vec3 sphereCenter, float sphereRadius)
{
	float distanceSquared = (point - sphereCenter).GetLengthSquared();
	return distanceSquared <= sphereRadius * sphereRadius;
}

bool IsPointInsideCylinderZ3D(Vec3 const& point, Vec2 const& cylinCenterXY, FloatRange const& cylinMinMaxZ, float cylinRadius)
{
	if (point.z < cylinMinMaxZ.m_min || point.z > cylinMinMaxZ.m_max)
	{
		return false;
	}

	Vec2 pointXY(point.x, point.y);
	float distanceSquared = (pointXY - cylinCenterXY).GetLengthSquared();
	return distanceSquared <= cylinRadius * cylinRadius;
}



bool IsPointInsideTriangle2D(Vec2 const& point, Vec2 const& triCCW0, Vec2 const& triCCW1, Vec2 const& triCCW2)
{
	float areaABC = (triCCW1.x - triCCW0.x) * (triCCW2.y - triCCW0.y) - (triCCW2.x - triCCW0.x) * (triCCW1.y - triCCW0.y);

	float areaPAB = (triCCW1.x - point.x) * (triCCW2.y - point.y) - (triCCW2.x - point.x) * (triCCW1.y - point.y);
	float areaPBC = (triCCW2.x - point.x) * (triCCW0.y - point.y) - (triCCW0.x - point.x) * (triCCW2.y - point.y);
	float areaPCA = (triCCW0.x - point.x) * (triCCW1.y - point.y) - (triCCW1.x - point.x) * (triCCW0.y - point.y);

	return (areaPAB * areaABC > 0.0f && areaPBC * areaABC > 0.0f && areaPCA * areaABC > 0.0f);
}

bool IsPointInsideTriangle2D(Vec2 const& point, Triangle2 triangle)
{
	return IsPointInsideTriangle2D(point, triangle.m_pointsCounterClockwise[0], 
		triangle.m_pointsCounterClockwise[1], triangle.m_pointsCounterClockwise[2]);
}

bool IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box)
{
	return (point.x >= box.m_mins.x && point.x <= box.m_maxs.x) &&
		(point.y >= box.m_mins.y && point.y <= box.m_maxs.y);
}

bool IsPointInsideAABB3D(Vec3 const& point, AABB3 const& box)
{
	return (point.x >= box.m_mins.x && point.x <= box.m_maxs.x &&
		point.y >= box.m_mins.y && point.y <= box.m_maxs.y &&
		point.z >= box.m_mins.z && point.z <= box.m_maxs.z);
}


bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& box)
{
	Vec2 localPoint = box.GetLocalPosForWorldPos(point);

	return (localPoint.x >= -box.m_halfDimensions.x && localPoint.x <= box.m_halfDimensions.x) &&
		(localPoint.y >= -box.m_halfDimensions.y && localPoint.y <= box.m_halfDimensions.y);
}

bool IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius)
{
	Vec2 nearestPoint = GetNearestPointOnLineSegment2D(point, boneStart, boneEnd);
	float distSquared = (point - nearestPoint).GetLengthSquared();

	return distSquared <= (radius * radius);
}

bool IsPointInsideCapsule2D(Vec2 const& point, Capsule2 capsule)
{
	return IsPointInsideCapsule2D(point, capsule.m_start, capsule.m_end, capsule.m_radius);
}

bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius)
{
	Vec2 toPoint = point - sectorTip;
	float distToPoint = toPoint.GetLength();

	if (distToPoint > sectorRadius)
	{
		return false;
	}

	Vec2 sectorForwardDirection(CosDegrees(sectorForwardDegrees), SinDegrees(sectorForwardDegrees));
	float angleToPoint = GetAngleDegreesBetweenVectors2D(sectorForwardDirection, toPoint);
	float halfApertureRadians = sectorApertureDegrees / 2;

	return angleToPoint <= halfApertureRadians;
}

bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius)
{
	Vec2 toPoint = point - sectorTip;
	float distToPoint = toPoint.GetLength();

	if (distToPoint > sectorRadius)
	{
		return false;
	}

	toPoint.Normalize();
	float dotProduct = DotProduct2D(toPoint, sectorForwardNormal);
	float halfApertureCosine = CosDegrees(sectorApertureDegrees / 2);

	return dotProduct >= halfApertureCosine;
}

bool IsPointInsideOBB3D(Vec3 const& point, Vec3 const& i, Vec3 const& j, Vec3 const& k, Vec3 const& halfDimensions, Vec3 const& center)
{
	Vec3 disp = point - center;

	float localX = DotProduct3D(disp, i);
	float localY = DotProduct3D(disp, j);
	float localZ = DotProduct3D(disp, k);

	return (fabsf(localX) <= halfDimensions.x) &&
		(fabsf(localY) <= halfDimensions.y) &&
		(fabsf(localZ) <= halfDimensions.z);
}

bool IsPointInFrontOfPlane3D(Vec3 const& point, Vec3 const& normal, float distFromOrigin)
{
	float signedDistance = DotProduct3D(point, normal) - distFromOrigin;
	return signedDistance > 0.f;
}

Vec2 GetNearestPointOnDisc2D(Vec2 const& referencePosition, Vec2 const& discCenter, float discRadius)
{
	Vec2 centerToPoint = referencePosition - discCenter;
	centerToPoint.ClampLength(discRadius);

	return discCenter + centerToPoint;
}

Vec2 GetNearestPointOnDisc2D(Vec2 const& referencePos, Disc2 disc)
{
	return GetNearestPointOnDisc2D(referencePos, disc.m_center, disc.m_radius);
}

Vec2 GetNearestPointOnAABB2D(Vec2 const& referencePos, AABB2 const& alignedBox)
{
	Vec2 nearestPoint;

	nearestPoint.x = GetClamped(referencePos.x, alignedBox.m_mins.x, alignedBox.m_maxs.x);
	nearestPoint.y = GetClamped(referencePos.y, alignedBox.m_mins.y, alignedBox.m_maxs.y);

	return nearestPoint;
}

Vec2 GetNearestPointOnOBB2D(Vec2 const& referencePos, OBB2 const& orientedBox)
{
	Vec2 localPoint = orientedBox.GetLocalPosForWorldPos(referencePos);

	Vec2 clampedLocalPoint;
	clampedLocalPoint.x = GetClamped(localPoint.x, -orientedBox.m_halfDimensions.x, orientedBox.m_halfDimensions.x);
	clampedLocalPoint.y = GetClamped(localPoint.y, -orientedBox.m_halfDimensions.y, orientedBox.m_halfDimensions.y);

	return orientedBox.GetWorldPosForLocalPos(clampedLocalPoint);
}

Vec2 GetNearestPointOnInfiniteLine2D(Vec2 const& referencePos, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine)
{
	Vec2 lineDirection = anotherPointOnLine - pointOnLine;
	Vec2 pointToLine = referencePos - pointOnLine;
	float projectionLength = DotProduct2D(pointToLine, lineDirection) / lineDirection.GetLengthSquared();
	Vec2 nearestPointOnLine = pointOnLine + lineDirection * projectionLength;

	return nearestPointOnLine;
}

Vec2 GetNearestPointOnLineSegment2D(Vec2 const& referencePos, Vec2 const& start, Vec2 const& end)
{
	Vec2 displacement = end - start;
	Vec2 normalizedDisplacement = displacement.GetNormalized();

	float projectionLength = DotProduct2D(referencePos - start, normalizedDisplacement);
	if (projectionLength < 0)
	{
		return start;
	}
	else if (projectionLength > displacement.GetLength())
	{
		return end;
	}

	return start + normalizedDisplacement * projectionLength;
}

Vec2 GetNearestPointOnLineSegment2D(Vec2 const& referencePos, LineSegment2 lineSegment)
{
	return GetNearestPointOnLineSegment2D(referencePos, lineSegment.m_start, lineSegment.m_end);
}

Vec2 GetNearestPointOnCapsule2D(Vec2 const& referencePos, Vec2 const& boneStart, Vec2 const& boneEnd, float radius)
{
	Vec2 lineDirection = boneEnd - boneStart;

	float lineLengthSquared = lineDirection.GetLengthSquared();

	if (lineLengthSquared == 0.0f) {
		return boneStart + Vec2(radius, 0);
	}

	Vec2 pointToStart = referencePos - boneStart;

	float t = DotProduct2D(pointToStart, lineDirection) / lineLengthSquared;

	if (t < 0.0f) {
		return boneStart + (referencePos - boneStart).GetNormalized() * radius;
	}
	else if (t > 1.0f) {
		return boneEnd + (referencePos - boneEnd).GetNormalized() * radius;
	}

	Vec2 nearestPointOnSegment = boneStart + lineDirection * t;

	Vec2 nearestPointOnCapsule = nearestPointOnSegment + (referencePos - nearestPointOnSegment).GetNormalized() * radius;

	return nearestPointOnCapsule;
}

Vec2 GetNearestPointOnCapsule2D(Vec2 const& referencePos, Capsule2 capsule)
{
	return GetNearestPointOnCapsule2D(referencePos, capsule.m_start, capsule.m_end, capsule.m_radius);
}

Vec2 GetNearestPointOnTriangle2D(Vec2 const& referencePos, Vec2 const& triCCW0, Vec2 const& triCCW1, Vec2 const& triCCW2)
{
	if (IsPointInsideTriangle2D(referencePos, triCCW0, triCCW1, triCCW2)) {
		return referencePos;
	}

	Vec2 nearestPointOnEdge01 = GetNearestPointOnLineSegment2D(referencePos, triCCW0, triCCW1);
	Vec2 nearestPointOnEdge12 = GetNearestPointOnLineSegment2D(referencePos, triCCW1, triCCW2);
	Vec2 nearestPointOnEdge20 = GetNearestPointOnLineSegment2D(referencePos, triCCW2, triCCW0);

	float distance01 = (referencePos - nearestPointOnEdge01).GetLengthSquared();
	float distance12 = (referencePos - nearestPointOnEdge12).GetLengthSquared();
	float distance20 = (referencePos - nearestPointOnEdge20).GetLengthSquared();

	if (distance01 <= distance12 && distance01 <= distance20) {
		return nearestPointOnEdge01;
	}
	else if (distance12 <= distance01 && distance12 <= distance20) {
		return nearestPointOnEdge12;
	}
	else {
		return nearestPointOnEdge20;
	}
}

Vec2 GetNearestPointOnTriangle2D(Vec2 const& referencePos, Triangle2 triangle)
{
	return GetNearestPointOnTriangle2D(referencePos, triangle.m_pointsCounterClockwise[0], triangle.m_pointsCounterClockwise[1], triangle.m_pointsCounterClockwise[2]);
}

Vec3 GetNearestPointOnAABB3D(Vec3 const& referencePos, Vec3 const& mins, Vec3 const& maxs)
{
	Vec3 nearestPoint;

	nearestPoint.x = (referencePos.x < mins.x) ? mins.x : (referencePos.x > maxs.x ? maxs.x : referencePos.x);
	nearestPoint.y = (referencePos.y < mins.y) ? mins.y : (referencePos.y > maxs.y ? maxs.y : referencePos.y);
	nearestPoint.z = (referencePos.z < mins.z) ? mins.z : (referencePos.z > maxs.z ? maxs.z : referencePos.z);

	return nearestPoint;
}

Vec3 GetNearestPointOnSphere3D(Vec3 const& referencePos, Vec3 const& center, float radius)
{
	Vec3 direction = referencePos - center;

	float distance = direction.GetLength();

	if (distance > radius)
	{
		Vec3 nearestPoint = center + direction * (radius / distance);
		return nearestPoint;
	}

	return referencePos;
}

Vec3 GetNearestPointOnZCylinder3D(Vec3 const& referencePos, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius)
{
	Vec2 referencePosXY(referencePos.x, referencePos.y);
	Vec2 centerToPosXY = referencePosXY - centerXY;

	float distanceXY = centerToPosXY.GetLength();
	Vec2 nearestXY;
	if (distanceXY > radius)
	{
		nearestXY = centerXY + centerToPosXY * (radius / distanceXY);
	}
	else
	{
		nearestXY = referencePosXY;
	}

	float nearestZ;
	if (referencePos.z < minMaxZ.m_min)
	{
		nearestZ = minMaxZ.m_min;
	}
	else if (referencePos.z > minMaxZ.m_max)
	{
		nearestZ = minMaxZ.m_max;
	}
	else
	{
		nearestZ = referencePos.z;
	}

	return Vec3(nearestXY.x, nearestXY.y, nearestZ);
}

Vec3 GetNearestPointOnOBB3D(Vec3 const& referencePos, Vec3 const& i, Vec3 const& j, Vec3 const& k, Vec3 const& halfDimensions, Vec3 const& center)
{
	Vec3 disp = referencePos - center;
	Vec3 nearest = center;

	float localX = DotProduct3D(disp, i);
	localX = GetClamped(localX, -halfDimensions.x, halfDimensions.x);
	nearest += localX * i;

	float localY = DotProduct3D(disp, j);
	localY = GetClamped(localY, -halfDimensions.y, halfDimensions.y);
	nearest += localY * j;

	float localZ = DotProduct3D(disp, k);
	localZ = GetClamped(localZ, -halfDimensions.z, halfDimensions.z);
	nearest += localZ * k;

	return nearest;
}

Vec3 GetNearestPointOnPlane3D(Vec3 const& referencePos, Vec3 const& normal, float distFromOrigin)
{
	float signedDistance = DotProduct3D(referencePos, normal) - distFromOrigin;

	return referencePos - signedDistance * normal;
}



Vec3 RotateVectorInPlane(Vec3 vec, float angleDegrees)
{
	Vec2 rotated = vec.GetXY().GetRotatedDegrees(angleDegrees);
	return Vec3(rotated.x, rotated.y, vec.z);
}

RaycastResult2D RaycastVsDisc2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 discCenter, float discRadius)
{
	RaycastResult2D result;
	Vec2 startToCenterDist = discCenter - startPos;

	if (startToCenterDist.GetLengthSquared() < discRadius * discRadius) {
		result.m_didImpact = true;
		result.m_impactDist = 0.f;
		result.m_impactPos = startPos;
		result.m_impactNormal = -fwdNormal;
		result.m_rayFwdNormal = fwdNormal;
		result.m_rayStartPos = startPos;
		result.m_rayMaxLength = maxDist;
		return result;
	}

	Vec2 leftNormal = fwdNormal.GetRotated90Degrees();
	float startLeftOffset = DotProduct2D(startToCenterDist, leftNormal);

	if (startLeftOffset >= discRadius || startLeftOffset <= -discRadius) {
		return result;
	}

	float startfwdOffset = DotProduct2D(startToCenterDist, fwdNormal);

	if (startfwdOffset >= maxDist + discRadius || startfwdOffset <= -discRadius) {
		return result;
	}

	float adjust = sqrtf(discRadius * discRadius - startLeftOffset * startLeftOffset);
	result.m_didImpact = true;
	result.m_impactDist = startfwdOffset - adjust;

	if (result.m_impactDist >= maxDist || result.m_impactDist <= 0) {
		result.m_didImpact = false;
		return result;
	}

	result.m_impactPos = startPos + fwdNormal * result.m_impactDist;
	result.m_impactNormal = (result.m_impactPos - discCenter).GetNormalized();

	result.m_rayFwdNormal = fwdNormal;
	result.m_rayStartPos = startPos;
	result.m_rayMaxLength = maxDist;

	return result;
}

RaycastResult2D RaycastVsDisc2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Disc2 disc)
{
	return RaycastVsDisc2D(startPos, fwdNormal, maxDist, disc.m_center, disc.m_radius);
}

RaycastResult2D RaycastVsLineSegment2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 lineStart, Vec2 lineEnd)
{
	RaycastResult2D result;
	result.m_rayFwdNormal = fwdNormal;
	result.m_rayStartPos = startPos;
	result.m_rayMaxLength = maxDist;

	Vec2 lineDir = lineEnd - lineStart;

	Vec2 rayDir = fwdNormal;

	float crossProduct = CrossProduct2D(rayDir, lineDir);

	Vec2 startToLineStart = lineStart - startPos;
	float t = (startToLineStart.x * lineDir.y - startToLineStart.y * lineDir.x) / crossProduct;
	float s = (startToLineStart.x * rayDir.y - startToLineStart.y * rayDir.x) / crossProduct;

	if (t >= 0 && s >= 0 && s <= 1 && t <= maxDist)
	{
		result.m_didImpact = true;
		result.m_impactPos = startPos + rayDir * t;
		result.m_impactDist = t;

		Vec2 lineNormal = lineDir.GetRotated90Degrees().GetNormalized();

		if (DotProduct2D(lineNormal, rayDir) > 0)
		{
			lineNormal = -lineNormal;
		}

		
		result.m_impactNormal = lineNormal;
	}

	return result;
}

RaycastResult2D RaycastVsLineSegment2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, LineSegment2 lineSegment)
{
	return RaycastVsLineSegment2D(startPos, fwdNormal, maxDist, lineSegment.m_start, lineSegment.m_end);
}

RaycastResult2D RaycastVsAABB2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 mins, Vec2 maxs)
{
	RaycastResult2D result;
	result.m_rayFwdNormal = fwdNormal;
	result.m_rayStartPos = startPos;
	result.m_rayMaxLength = maxDist;

	bool isInsideAABB = (startPos.x >= mins.x && startPos.x <= maxs.x) &&
		(startPos.y >= mins.y && startPos.y <= maxs.y);

	if (isInsideAABB)
	{
		result.m_didImpact = true;
		result.m_impactDist = 0.f;
		result.m_impactPos = startPos;
		result.m_impactNormal = -fwdNormal;
		return result;
	}

	Vec2 invDir(1.f / fwdNormal.x, 1.f / fwdNormal.y);

	float tNearX = (mins.x - startPos.x) * invDir.x;
	float tFarX = (maxs.x - startPos.x) * invDir.x;
	float tNearY = (mins.y - startPos.y) * invDir.y;
	float tFarY = (maxs.y - startPos.y) * invDir.y;

	if (tNearX > tFarX) std::swap(tNearX, tFarX);
	if (tNearY > tFarY) std::swap(tNearY, tFarY);

	float tNear = std::max(tNearX, tNearY);
	float tFar = std::min(tFarX, tFarY);

	if (tNear <= tFar && tFar >= 0 && tNear <= maxDist)
	{
		result.m_didImpact = true;
		result.m_impactDist = tNear; 
		result.m_impactPos = startPos + fwdNormal * tNear; 

		if (tNearX > tNearY)
		{
			if (fwdNormal.x > 0)
				result.m_impactNormal = Vec2(-1.f, 0.f);
			else
				result.m_impactNormal = Vec2(1.f, 0.f);
		}
		else
		{
			if (fwdNormal.y > 0)
				result.m_impactNormal = Vec2(0.f, -1.f);
			else
				result.m_impactNormal = Vec2(0.f, 1.f);
		}
	}

	return result;
}

RaycastResult2D RaycastVsAABB2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, AABB2 bound)
{
	return RaycastVsAABB2D(startPos,fwdNormal,maxDist, bound.m_mins, bound.m_maxs);
}

float NormalizeByte(unsigned char b)
{
	return static_cast<float>(b) / 255.f;
}

unsigned char DenormalizeByte(float zeroToOne)
{
	if (zeroToOne <= 0.f) return 0;
	if (zeroToOne >= 1.f) return 255;

	return static_cast<unsigned char>(zeroToOne * 256.f);
}

Mat44 GetBillboardMatrix(BillboardType billboardType, Mat44 const& cameraMatrix, const Vec3& position, const Vec2& scale /*= Vec2(1.0f, 1.0f)*/)
{
	Mat44 matrix;
	Vec3 cameraPosition = cameraMatrix.GetTranslation3D();
	Vec3 toCamera = cameraPosition - position;

	if (billboardType == BillboardType::FULL_FACING)
	{
		Vec3 forward = toCamera;
		forward = forward / forward.GetLength();
		Vec3 up = Vec3(0.f, 0.f, 1.f);
		Vec3 right = CrossProduct3D(up, forward);
		right = right / right.GetLength();
		up = CrossProduct3D(forward, right);

		matrix.SetIJK3D(forward, right, up);
	}
	else if (billboardType == BillboardType::WORLD_UP_FACING)
	{
		Vec3 forward = toCamera;
		forward = forward / forward.GetLength();
		Vec3 up = Vec3(0.f, 0.f, 1.f);
		Vec3 right = CrossProduct3D(up, forward);
		right = right / right.GetLength();
		up = CrossProduct3D(forward, right);

		matrix.SetIJK3D(forward, right, up);
	}
	else if (billboardType == BillboardType::WORLD_UP_OPPOSING)
	{
		Vec3 forward = -cameraMatrix.GetIBasis3D();
		forward = forward / forward.GetLength();
		Vec3 up = Vec3(0.f, 0.f, 1.f);
		Vec3 right = CrossProduct3D(up, forward);
		right = right / right.GetLength();
		up = CrossProduct3D(forward, right);

		matrix.SetIJK3D(forward, right, up);
	}
	else if (billboardType == BillboardType::FULL_OPPOSING)
	{
		Vec3 forward = -cameraMatrix.GetIBasis3D();
		Vec3 right = -cameraMatrix.GetJBasis3D();
		Vec3 up = cameraMatrix.GetKBasis3D();

		matrix.SetIJK3D(forward, right, up);
	}
	else if (billboardType == BillboardType::NONE)
	{
		return matrix;
	}

	matrix.SetTranslation3D(position);
	matrix.AppendScaleNonUniform2D(scale);

	return matrix;
}

RaycastResult3D RaycastVsAABB3D(Vec3 rayStart, Vec3 rayForwardNormal, float rayLength, AABB3 box)
{
	RaycastResult3D result;
	result.m_didImpact = false;
	result.m_rayStart = rayStart;
	result.m_rayForwardNormal = rayForwardNormal;
	result.m_rayMaxLength = rayLength;

	Vec3 rayDir = rayForwardNormal;

	if (IsPointInsideAABB3D(rayStart, box))
	{
		result.m_didImpact = true;
		result.m_impactPos = rayStart;
		result.m_impactDist = 0.0f;
		result.m_impactNormal = -rayForwardNormal;
		return result;
	}

	float tMin = 0.0f;
	float tMax = rayLength;

	for (int axis = 0; axis < 3; ++axis)
	{
		float rayStartAxis, rayDirAxis, boxMinAxis, boxMaxAxis;

		if (axis == 0)
		{
			rayStartAxis = rayStart.x;
			rayDirAxis = rayDir.x;
			boxMinAxis = box.m_mins.x;
			boxMaxAxis = box.m_maxs.x;
		}
		else if (axis == 1)
		{
			rayStartAxis = rayStart.y;
			rayDirAxis = rayDir.y;
			boxMinAxis = box.m_mins.y;
			boxMaxAxis = box.m_maxs.y;
		}
		else
		{
			rayStartAxis = rayStart.z;
			rayDirAxis = rayDir.z;
			boxMinAxis = box.m_mins.z;
			boxMaxAxis = box.m_maxs.z;
		}

		if (fabsf(rayDirAxis) < 0.0001f)
		{
			if (rayStartAxis < boxMinAxis || rayStartAxis > boxMaxAxis)
			{
				return result;
			}
		}
		else
		{
			float t1 = (boxMinAxis - rayStartAxis) / rayDirAxis;
			float t2 = (boxMaxAxis - rayStartAxis) / rayDirAxis;

			if (t1 > t2)
			{
				std::swap(t1, t2);
			}

			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax)
			{
				return result;
			}
		}
	}

	if (tMin >= 0.0f && tMin <= rayLength)
	{
		result.m_didImpact = true;
		result.m_impactPos = rayStart + rayDir * tMin;
		result.m_impactDist = tMin;

		if (tMin == (box.m_mins.x - rayStart.x) / rayDir.x) result.m_impactNormal = Vec3(-1.0f, 0.0f, 0.0f);
		else if (tMin == (box.m_maxs.x - rayStart.x) / rayDir.x) result.m_impactNormal = Vec3(1.0f, 0.0f, 0.0f);
		else if (tMin == (box.m_mins.y - rayStart.y) / rayDir.y) result.m_impactNormal = Vec3(0.0f, -1.0f, 0.0f);
		else if (tMin == (box.m_maxs.y - rayStart.y) / rayDir.y) result.m_impactNormal = Vec3(0.0f, 1.0f, 0.0f);
		else if (tMin == (box.m_mins.z - rayStart.z) / rayDir.z) result.m_impactNormal = Vec3(0.0f, 0.0f, -1.0f);
		else if (tMin == (box.m_maxs.z - rayStart.z) / rayDir.z) result.m_impactNormal = Vec3(0.0f, 0.0f, 1.0f);
	}

	return result;
}


RaycastResult3D RaycastVsSphere3D(Vec3 rayStart, Vec3 rayForwardNormal, float rayLength, Vec3 sphereCenter, float sphereRadius)
{
	RaycastResult3D result;
	result.m_didImpact = false;
	result.m_rayStart = rayStart;
	result.m_rayForwardNormal = rayForwardNormal;
	result.m_rayMaxLength = rayLength;

	if (IsPointInsideSphere3D(rayStart, sphereCenter, sphereRadius))
	{
		result.m_didImpact = true;
		result.m_impactPos = rayStart;
		result.m_impactDist = 0.0f;
		result.m_impactNormal = -rayForwardNormal;
		return result;
	}

	Vec3 rayToSphere = sphereCenter - rayStart;

	float projectionLength = DotProduct3D(rayToSphere, rayForwardNormal);

	float distanceSquared = rayToSphere.GetLengthSquared() - projectionLength * projectionLength;

	if (distanceSquared > sphereRadius * sphereRadius)
	{
		return result;
	}

	float halfChordLength = sqrtf(sphereRadius * sphereRadius - distanceSquared);
	float t1 = projectionLength - halfChordLength;
	float t2 = projectionLength + halfChordLength;

	if (t1 > rayLength || t2 < 0.0f)
	{
		return result;
	}

	float t = (t1 < 0.0f) ? t2 : t1;

	if (t < 0.0f || t > rayLength)
	{
		return result;
	}

	result.m_didImpact = true;
	result.m_impactPos = rayStart + rayForwardNormal * t;
	result.m_impactDist = t;

	result.m_impactNormal = (result.m_impactPos - sphereCenter).GetNormalized();

	return result;
}



RaycastResult3D RaycastVsCylinderZ3D(Vec3 rayStart, Vec3 rayForwardNormal, float rayLength, Vec2 const& centerXY, FloatRange const& minMaxZ, float radiusXY)
{
	RaycastResult3D result;
	result.m_didImpact = false;
	result.m_rayStart = rayStart;
	result.m_rayForwardNormal = rayForwardNormal;
	result.m_rayMaxLength = rayLength;

	if (IsPointInsideCylinderZ3D(rayStart, centerXY, minMaxZ, radiusXY))
	{
		result.m_didImpact = true;
		result.m_impactPos = rayStart;
		result.m_impactDist = 0.0f;
		result.m_impactNormal = -rayForwardNormal;
		return result;
	}

	Vec2 rayStartXY(rayStart.x, rayStart.y);
	Vec2 rayDirXY(rayForwardNormal.x, rayForwardNormal.y);
	Vec2 centerToRayStartXY = rayStartXY - centerXY;

	float rayDirLengthSquared = rayDirXY.GetLengthSquared();
	float rayToCenterDotDir = 2.0f * DotProduct2D(centerToRayStartXY, rayDirXY);
	float centerToRayStartLengthSquared = centerToRayStartXY.GetLengthSquared();
	float radiusSquared = radiusXY * radiusXY;

	if (rayForwardNormal.z != 0.0f)
	{
		float tBottom = (minMaxZ.m_min - rayStart.z) / rayForwardNormal.z;
		Vec3 bottomPos = rayStart + rayForwardNormal * tBottom;
		Vec2 bottomPosXY(bottomPos.x, bottomPos.y);

		if (tBottom >= 0.0f && tBottom <= rayLength && (bottomPosXY - centerXY).GetLengthSquared() <= radiusSquared)
		{
			result.m_didImpact = true;
			result.m_impactPos = bottomPos;
			result.m_impactDist = tBottom;
			result.m_impactNormal = Vec3(0.0f, 0.0f, -1.0f);
			return result;
		}

		float tTop = (minMaxZ.m_max - rayStart.z) / rayForwardNormal.z;
		Vec3 topPos = rayStart + rayForwardNormal * tTop;
		Vec2 topPosXY(topPos.x, topPos.y);

		if (tTop >= 0.0f && tTop <= rayLength && (topPosXY - centerXY).GetLengthSquared() <= radiusSquared)
		{
			result.m_didImpact = true;
			result.m_impactPos = topPos;
			result.m_impactDist = tTop;
			result.m_impactNormal = Vec3(0.0f, 0.0f, 1.0f);
			return result;
		}
	}

	float discriminant = rayToCenterDotDir * rayToCenterDotDir - 4.0f * rayDirLengthSquared * (centerToRayStartLengthSquared - radiusSquared);
	if (discriminant < 0.0f) {
		return result;
	}

	float sqrtDiscriminant = sqrtf(discriminant);
	float tNear = (-rayToCenterDotDir - sqrtDiscriminant) / (2.0f * rayDirLengthSquared);
	float tFar = (-rayToCenterDotDir + sqrtDiscriminant) / (2.0f * rayDirLengthSquared);

	if (tNear > rayLength || tFar < 0.0f) {
		return result;
	}

	float t = tNear;
	if (tNear < 0.0f) {
		t = tFar;
	}

	if (t < 0.0f || t > rayLength) {
		return result;
	}

	Vec3 impactPos = rayStart + rayForwardNormal * t;
	float impactZ = impactPos.z;

	if (impactZ < minMaxZ.m_min || impactZ > minMaxZ.m_max) {
		return result;
	}

	result.m_didImpact = true;
	result.m_impactPos = impactPos;
	result.m_impactDist = t;

	Vec2 impactXY(impactPos.x, impactPos.y);
	Vec2 normalXY = (impactXY - centerXY).GetNormalized();
	result.m_impactNormal = Vec3(normalXY.x, normalXY.y, 0.0f);

	return result;
}

RaycastResult3D RaycastVsOBB3D(Vec3 rayStart, Vec3 rayForwardNormal, float rayLength,
	Vec3 const& obbI, Vec3 const& obbJ, Vec3 const& obbK,
	Vec3 const& obbHalfDimensions, Vec3 const& obbCenter)
{
	RaycastResult3D result;
	result.m_rayStart = rayStart;
	result.m_rayForwardNormal = rayForwardNormal;
	result.m_rayMaxLength = rayLength;

	Mat44 worldToLocal;
	worldToLocal.SetIJKT3D(obbI, obbJ, obbK, obbCenter);
	worldToLocal = worldToLocal.GetOrthonormalInverse();

	Vec3 localRayStart = worldToLocal.TransformPosition3D(rayStart);
	Vec3 localRayDir = worldToLocal.TransformVectorQuantity3D(rayForwardNormal);

	AABB3 localBounds = AABB3(-obbHalfDimensions, obbHalfDimensions);

	RaycastResult3D localResult = RaycastVsAABB3D(localRayStart, localRayDir, rayLength, localBounds);

	if (!localResult.m_didImpact)
		return result;

	Mat44 localToWorld;
	localToWorld.SetIJKT3D(obbI, obbJ, obbK, obbCenter); 

	result.m_didImpact = true;
	result.m_impactDist = localResult.m_impactDist;
	result.m_impactPos = localToWorld.TransformPosition3D(localResult.m_impactPos);
	result.m_impactNormal = localToWorld.TransformVectorQuantity3D(localResult.m_impactNormal).GetNormalized();

	return result;
}

RaycastResult3D RaycastVsPlane3D(Vec3 rayStart, Vec3 rayForwardNormal, float rayLength, Vec3 const& planeNormal, float planeDistFromOrigin)
{
	RaycastResult3D result;
	result.m_rayStart = rayStart;
	result.m_rayForwardNormal = rayForwardNormal;
	result.m_rayMaxLength = rayLength;

	float dot = DotProduct3D(rayForwardNormal, planeNormal);

	if (dot == 0.f)
	{
		return result;
	}

	float t = (planeDistFromOrigin - DotProduct3D(rayStart, planeNormal)) / dot;

	if (t < 0.f || t > rayLength)
	{
		return result;
	}

	result.m_didImpact = true;
	result.m_impactDist = t;
	result.m_impactPos = rayStart + t * rayForwardNormal;
	result.m_impactNormal = dot > 0.f ? -planeNormal : planeNormal;

	return result;
}


float ComputeCubicBezier1D(float A, float B, float C, float D, float t)
{
	float AB = A + (B - A) * t;
	float BC = B + (C - B) * t;
	float CD = C + (D - C) * t;

	float ABC = AB + (BC - AB) * t;
	float BCD = BC + (CD - BC) * t;

	return ABC + (BCD - ABC) * t;
}

float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float t)
{
	float AB = A + (B - A) * t;
	float BC = B + (C - B) * t;
	float CD = C + (D - C) * t;
	float DE = D + (E - D) * t;
	float EF = E + (F - E) * t;

	float ABC = AB + (BC - AB) * t;
	float BCD = BC + (CD - BC) * t;
	float CDE = CD + (DE - CD) * t;
	float DEF = DE + (EF - DE) * t;

	float ABCD = ABC + (BCD - ABC) * t;
	float BCDE = BCD + (CDE - BCD) * t;
	float CDEF = CDE + (DEF - CDE) * t;

	float ABCDE = ABCD + (BCDE - ABCD) * t;
	float BCDEF = BCDE + (CDEF - BCDE) * t;

	return ABCDE + (BCDEF - ABCDE) * t;
}



bool PushDiscOutOfPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint)
{
	Vec2 toFixedPoint = fixedPoint - mobileDiscCenter;
	float distanceToFixedPoint = toFixedPoint.GetLength();

	if (distanceToFixedPoint >= discRadius)
	{
		return false;
	}

	float overlap = discRadius - distanceToFixedPoint;
	Vec2 pushDirection = toFixedPoint / distanceToFixedPoint;
	mobileDiscCenter = mobileDiscCenter - pushDirection * overlap;

	return true;
}

bool PushDiscOutOfDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius)
{
	Vec2 toFixedDisc = fixedDiscCenter - mobileDiscCenter;
	float distanceBetweenDiscs = toFixedDisc.GetLength();
	float totalRadius = mobileDiscRadius + fixedDiscRadius;

	if (distanceBetweenDiscs >= totalRadius)
	{
		return false;
	}

	float overlap = totalRadius - distanceBetweenDiscs;
	Vec2 pushDirection = toFixedDisc / distanceBetweenDiscs;
	mobileDiscCenter = mobileDiscCenter - pushDirection * overlap;

	return true;
}

bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius)
{
	Vec2 toB = bCenter - aCenter;
	float distanceBetweenDiscs = toB.GetLength();
	float totalRadius = aRadius + bRadius;

	if (distanceBetweenDiscs >= totalRadius)
	{
		return false;
	}

	float overlap = totalRadius - distanceBetweenDiscs;
	Vec2 pushDirection = toB / distanceBetweenDiscs;

	aCenter = aCenter - pushDirection * (overlap / 2);
	bCenter = bCenter + pushDirection * (overlap / 2);

	return true;
}

bool PushDiscOutOfAABB2D(Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox)
{
	Vec2 nearestPoint = mobileDiscCenter;

	if (mobileDiscCenter.x < fixedBox.m_mins.x)
		nearestPoint.x = fixedBox.m_mins.x;
	else if (mobileDiscCenter.x > fixedBox.m_maxs.x)
		nearestPoint.x = fixedBox.m_maxs.x;

	if (mobileDiscCenter.y < fixedBox.m_mins.y)
		nearestPoint.y = fixedBox.m_mins.y;
	else if (mobileDiscCenter.y > fixedBox.m_maxs.y)
		nearestPoint.y = fixedBox.m_maxs.y;

	Vec2 toNearest = nearestPoint - mobileDiscCenter;
	float distanceToNearest = toNearest.GetLength();

	if (distanceToNearest >= discRadius)
	{
		return false;
	}

	float overlap = discRadius - distanceToNearest;
	Vec2 pushDirection = toNearest / distanceToNearest;

	mobileDiscCenter = mobileDiscCenter - pushDirection * overlap;

	return true;
}


void BounceDiscOutOfDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2& mobileDiscVelocity, float mobileDiscElasticity,
	Vec2 const& fixedDiscCenter, float fixedDiscRadius, float fixedDiscElasticity)
{
	Vec2 displacement = mobileDiscCenter - fixedDiscCenter;
	Vec2 collisionNormal = displacement.GetNormalized();

	float distance = displacement.GetLength();
	float penetration = (mobileDiscRadius + fixedDiscRadius) - distance;
	mobileDiscCenter += collisionNormal * penetration;

	Vec2 V = mobileDiscVelocity;
	float VnMag = DotProduct2D(V, collisionNormal);
	Vec2 Vn = VnMag * collisionNormal;
	Vec2 Vt = V - Vn;

	float combinedElasticity = (mobileDiscElasticity + fixedDiscElasticity) * 0.5f;
	mobileDiscVelocity = Vt - Vn * combinedElasticity;
}



void BounceDiscOutOfOBB2D(Vec2& discCenter, float discRadius, Vec2& discVelocity, float discElasticity,
	Vec2 const& obbCenter, Vec2 const& obbIBasis, Vec2 const& obbHalfDimensions, float obbElasticity)
{
	OBB2 obb(obbCenter, obbIBasis, obbHalfDimensions);
	Vec2 nearestPoint = GetNearestPointOnOBB2D(discCenter, obb);
	Vec2 displacement = discCenter - nearestPoint;
	float dist = displacement.GetLength();
	if (dist < discRadius)
	{
		Vec2 normal = (dist > 0.0001f) ? (displacement / dist) : Vec2(1.f, 0.f);
		float penetration = discRadius - dist;
		discCenter += normal * penetration;
		float elasticity = (discElasticity + obbElasticity) * 0.5f;
		Vec2 v = discVelocity;
		float vn = DotProduct2D(v, normal); 
		Vec2 vN = vn * normal;
		Vec2 vT = v - vN; 
		discVelocity = vT - vN * elasticity; 
	}
}





void BounceDiscOutOfCapsule2D(Vec2& discCenter, float discRadius, Vec2& discVelocity, float discElasticity,
	Vec2 const& capsuleStart, Vec2 const& capsuleEnd, float capsuleRadius, float capsuleElasticity)
{
	Vec2 nearestPoint = GetNearestPointOnLineSegment2D(discCenter, capsuleStart, capsuleEnd);
	Vec2 displacement = discCenter - nearestPoint;

	float dist = displacement.GetLength();
	Vec2 normal = (dist > 0.0001f) ? (displacement / dist) : Vec2(1.f, 0.f);
	float penetration = (discRadius + capsuleRadius) - dist;
	discCenter += normal * penetration;

	float elasticity = (discElasticity + capsuleElasticity) * 0.5f;
	Vec2 v = discVelocity;
	float vn = DotProduct2D(v, normal);
	Vec2 vN = vn * normal;
	Vec2 vT = v - vN;
	discVelocity = vT - vN * elasticity;
}


void BounceDiscOutOfLineSegment2D(Vec2& discCenter, float discRadius, Vec2& discVelocity, float discElasticity,
	Vec2 const& lineStart, Vec2 const& lineEnd, float lineElasticity)
{
	Vec2 nearestPoint = GetNearestPointOnLineSegment2D(discCenter, lineStart, lineEnd);
	Vec2 displacement = discCenter - nearestPoint;

	float dist = displacement.GetLength();
	Vec2 normal = (dist > 0.0001f) ? (displacement / dist) : Vec2(1.f, 0.f);
	float penetration = discRadius - dist;
	discCenter += normal * penetration;

	float elasticity = (discElasticity + lineElasticity) * 0.5f;
	Vec2 v = discVelocity;
	float vn = DotProduct2D(v, normal);
	Vec2 vN = vn * normal;
	Vec2 vT = v - vN;
	discVelocity = vT - vN * elasticity;
}


void BounceDiscsOutOfEachOther(Vec2& discCenterA, float discRadiusA, Vec2& discVelocityA, float discElasticityA,
	Vec2& discCenterB, float discRadiusB, Vec2& discVelocityB, float discElasticityB)
{
	Vec2 displacement = discCenterA - discCenterB;
	float dist = displacement.GetLength();
	Vec2 normal = (dist > 0.0001f) ? (displacement / dist) : Vec2(1.f, 0.f);
	float penetration = (discRadiusA + discRadiusB) - dist;

	discCenterA += normal * (penetration * 0.5f);
	discCenterB -= normal * (penetration * 0.5f);

	Vec2 relativeVelocity = discVelocityA - discVelocityB;
	float approachSpeed = DotProduct2D(relativeVelocity, normal);
	if (approachSpeed >= 0.f)
		return;

	float elasticity = (discElasticityA + discElasticityB) * 0.5f;
	Vec2 impulse = -(1.f + elasticity) * approachSpeed * normal;
	discVelocityA += impulse * 0.5f;
	discVelocityB -= impulse * 0.5f;
}





