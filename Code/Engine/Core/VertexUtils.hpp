#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include <vector>

struct OBB2;
struct OBB3;
struct LineSegment2;
struct Capsule2;
struct Triangle2;
struct Disc2;
class FloatRange;

//typedef std::vector<Vertex_PCU> VertexArray;
const float PI = 3.1415926535897f;


void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float scaleXY, float rotationDegreesAboutZ, Vec2 const translationXY);
void TransformVertexArray3D(std::vector<Vertex_PCU>& verts, const Mat44& transform);

void TransformVertexArray3D(std::vector<Vertex_PCUTBN>& verts, const Mat44& transform);
AABB2 GetVertexBounds2D(const std::vector<Vertex_PCU>& verts);



void AddVertsForDisc2D(std::vector<Vertex_PCU>& verts, Vec2 const& discCenter, float discRadius, Rgba8 const& color);
void AddVertsForDisc2D(std::vector<Vertex_PCU>& verts, Disc2 const& disc, Rgba8 const& color);
void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, float thickness, Rgba8 const& color);
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& alignedBox, Rgba8 const& color);
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs);

void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts,
	const AABB3& bounds, const Rgba8& color = Rgba8::WHITE,
	const AABB2& UVs = AABB2::ZERO_TO_ONE);

void AddVertsForAABB3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes,
	const AABB3& bounds, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);

void AddVertsForAABB3DWall(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes,
	const AABB3& bounds, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForAABBWireframe3D(std::vector<Vertex_PCU>& verts, 
	AABB3 const& bounds, float lineThickness, 
	Rgba8 const& tint = Rgba8::WHITE);


void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& orientedBox, Rgba8 const& color);

void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color);
void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Capsule2 const& capsule, Rgba8 const& color);

void AddVertsForTriangle2D(std::vector<Vertex_PCU>& verts, Vec2 const& ccw0, Vec2 const& ccw1, Vec2 const& ccw2, Rgba8 const& color);
void AddVertsForTriangle2D(std::vector<Vertex_PCU>& verts, Triangle2 const& triangle, Rgba8 const& color);

void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color);
void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, LineSegment2 const& lineSeg, float thickness, Rgba8 const& color);
void AddVertsForLineSegment3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float thickness, Rgba8 const& color);
void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 const& color);

void AddVertsForQuad2D(std::vector<Vertex_PCU>& verts, Vec2 ccw0, Vec2 ccw1, Vec2 ccw2, Vec2 ccw3, Rgba8 tint, Vec2 uv0, Vec2 uv1, Vec2 uv2, Vec2 uv3);
void AddVertsForQuad2D(std::vector<Vertex_PCU>& verts, Vec2 ccw0, Vec2 ccw1, Vec2 ccw2, Vec2 ccw3, Rgba8 tint, AABB2 const& UVS = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts,
	const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft,
	Vec2 uv0, Vec2 uv1, Vec2 uv2, Vec2 uv3, const Rgba8& color = Rgba8::WHITE);
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, 
	const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, 
	const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);

void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes,
	const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft,
	const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);

void AddVertsForRoundedQuad3D(std::vector<Vertex_PCUTBN>& verts, 
	const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color, const AABB2& UVs);

void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts,
	const Vec3& center, float radius, const Rgba8& color = Rgba8::WHITE,
	const AABB2& UVs = AABB2::ZERO_TO_ONE, int numSlices = 32, int numStacks = 16);

void AddVertsForSphere3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices,
	const Vec3& center, float radius, const Rgba8& color = Rgba8::WHITE,
	const AABB2& UVs = AABB2::ZERO_TO_ONE, int numSlices = 32, int numStacks = 16);

void AddVertsForSkySphere3D(std::vector<Vertex_PCU>& verts, const Rgba8& color = Rgba8::WHITE, int numSlices = 32, int numStacks = 16);

void AddVertsForUVSphereZWireframe3D(std::vector<Vertex_PCU>& verts, 
	Vec3 const& center, float radius, float numStacks, float lineThickness, 
	Rgba8 const& tint = Rgba8::WHITE);

void AddVertsForArrow3D(std::vector<Vertex_PCU>& verts,
	const Vec3& start, const Vec3& end, float radius, float shaftPercentage, const Rgba8& color = Rgba8::WHITE);

void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts,
	const Vec3& start, const Vec3& end, float radius,
	const Rgba8& color = Rgba8::WHITE,
	const AABB2& UVs = AABB2::ZERO_TO_ONE,
	int numSlices = 8);

void AddVertsForCylinderZ3D(std::vector<Vertex_PCU>& verts, 
	Vec2 const& centerXY, FloatRange const& minMaxZ, 
	float radius, int numSlices, Rgba8 const& tint = Rgba8::WHITE, 
	AABB2 const& UVs = AABB2::ZERO_TO_ONE);

void AddVertsForCylinderZ3D(std::vector<Vertex_PCUTBN>& verts,
	std::vector<unsigned int>& indices, Vec2 const& centerXY, 
	FloatRange const& minMaxZ, float radius, int numSlices, Rgba8 const& tint = Rgba8::WHITE,
	AABB2 const& UVs = AABB2::ZERO_TO_ONE);

void AddVertsForCylinderZWireframe3D(std::vector<Vertex_PCU>& verts, 
	Vec2 const& centerXY, FloatRange const& minMaxZ, 
	float radius, int numSlices, float lineThickness, 
	Rgba8 const& tint = Rgba8::WHITE);

void AddVertsForOBB3D(std::vector<Vertex_PCU>& verts,
	Vec3 const& i, Vec3 const& j, Vec3 const& k,
	Vec3 const& halfDimensions, Vec3 const& center,
	const Rgba8& color = Rgba8::WHITE,
	const AABB2& UVs = AABB2::ZERO_TO_ONE);

void AddVertsForOBB3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices,
	OBB3 obb, const Rgba8& color = Rgba8::WHITE,
	const AABB2& UVs = AABB2::ZERO_TO_ONE);

void AddVertsForWireframeOBB3D(std::vector<Vertex_PCU>& verts,
	Vec3 const& i, Vec3 const& j, Vec3 const& k,
	Vec3 const& halfDimensions, Vec3 const& center,
	float lineThickness,
	const Rgba8& color = Rgba8::WHITE,
	const AABB2& UVs = AABB2::ZERO_TO_ONE);


void AddVertsForPlane3D(std::vector<Vertex_PCU>& verts,
	Vec3 const& normal, float distFromOrigin,
	const Rgba8& color = Rgba8::WHITE,
	const AABB2& UVs = AABB2::ZERO_TO_ONE);

void AddVertsForCone3D(std::vector<Vertex_PCU>& verts,
	const Vec3& start, const Vec3& end, float radius,
	const Rgba8& color = Rgba8::WHITE,
	const AABB2& UVs = AABB2::ZERO_TO_ONE,
	int numSlices = 8);

void AddVertsForPyramidArrow3D(std::vector<Vertex_PCU>& verts,
	const Vec3& start, const Vec3& end, float radius, const Rgba8& color = Rgba8::WHITE);
