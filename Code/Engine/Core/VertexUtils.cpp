#include "VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/Disc2.hpp"
#include "Engine/Math/Triangle2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/OBB3.hpp"

void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float scaleXY, float rotationDegreesAboutZ, Vec2 const translationXY)
{
	for (int vertIndex = 0; vertIndex < numVerts; ++vertIndex)
	{
		Vec3& pos = verts[vertIndex].m_position;
		TransformPositionXY3D(pos, scaleXY, rotationDegreesAboutZ, translationXY);
	}
}

void TransformVertexArray3D(std::vector<Vertex_PCU>& verts, const Mat44& transform)
{
	for (auto& vertex : verts)
	{
		vertex.m_position = transform.TransformPosition3D(vertex.m_position);
	}
}

void TransformVertexArray3D(std::vector<Vertex_PCUTBN>& verts, const Mat44& transform)
{
	for (auto& vertex : verts)
	{
		vertex.m_position = transform.TransformPosition3D(vertex.m_position);
	}
}

AABB2 GetVertexBounds2D(const std::vector<Vertex_PCU>& verts)
{
	Vec2 minBounds(verts[0].m_position.x, verts[0].m_position.y);
	Vec2 maxBounds(verts[0].m_position.x, verts[0].m_position.y);

	for (const auto& vert : verts) 
	{
		Vec2 pos(vert.m_position.x, vert.m_position.y);
		minBounds.x = std::min(minBounds.x, pos.x);
		minBounds.y = std::min(minBounds.y, pos.y);
		maxBounds.x = std::max(maxBounds.x, pos.x);
		maxBounds.y = std::max(maxBounds.y, pos.y);
	}

	return AABB2(minBounds, maxBounds);
}

void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8& color, const AABB2& UVs, int numSlices)
{
	Vec3 axis = (end - start).GetNormalized();

	Vec3 up = fabsf(axis.z) < 0.99f ? Vec3(0.f, 0.f, 1.f) : Vec3(0.f, 1.f, 0.f);

	Vec3 right = CrossProduct3D(up, axis).GetNormalized();
	Vec3 forward = CrossProduct3D(axis, right).GetNormalized();


	float deltaAngle = 360.f / static_cast<float>(numSlices);
	float uRange = UVs.m_maxs.x - UVs.m_mins.x;

	std::vector<Vec3> bottomVerts, topVerts;
	std::vector<Vec2> bottomUVs, topUVs;

	for (int i = 0; i <= numSlices; ++i)
	{
		float angle = deltaAngle * i;
		float cosA = CosDegrees(angle);
		float sinA = SinDegrees(angle);

		Vec3 offset = (right * cosA + forward * sinA) * radius;

		bottomVerts.push_back(start + offset);
		topVerts.push_back(end + offset);

		float u = UVs.m_mins.x + (uRange * i / numSlices);
		bottomUVs.push_back(Vec2(u, UVs.m_mins.y));
		topUVs.push_back(Vec2(u, UVs.m_maxs.y));
	}

	for (int i = 0; i < numSlices; ++i)
	{
		int next = i + 1;

		verts.push_back(Vertex_PCU(bottomVerts[i], color, bottomUVs[i]));
		verts.push_back(Vertex_PCU(bottomVerts[next], color, bottomUVs[next]));
		verts.push_back(Vertex_PCU(topVerts[i], color, topUVs[i]));

		verts.push_back(Vertex_PCU(topVerts[i], color, topUVs[i]));
		verts.push_back(Vertex_PCU(bottomVerts[next], color, bottomUVs[next]));
		verts.push_back(Vertex_PCU(topVerts[next], color, topUVs[next]));
	}

	Vec3 bottomCenter = start;
	Vec2 bottomCenterUV = Vec2(0.5f, 0.5f);
	for (int i = 0; i < numSlices; ++i)
	{
		int next = (i + 1) % numSlices;

		Vec2 bottomUV1 = Vec2(0.5f + 0.5f * CosDegrees(deltaAngle * i), 0.5f + 0.5f * SinDegrees(deltaAngle * i));
		Vec2 bottomUV2 = Vec2(0.5f + 0.5f * CosDegrees(deltaAngle * next), 0.5f + 0.5f * SinDegrees(deltaAngle * next));

		verts.push_back(Vertex_PCU(bottomCenter, color, bottomCenterUV));
		verts.push_back(Vertex_PCU(bottomVerts[next], color, bottomUV2));
		verts.push_back(Vertex_PCU(bottomVerts[i], color, bottomUV1));
	}

	Vec3 topCenter = end;
	Vec2 topCenterUV = Vec2(0.5f, 0.5f);
	for (int i = 0; i < numSlices; ++i)
	{
		int next = (i + 1) % numSlices;

		Vec2 topUV1 = Vec2(0.5f + 0.5f * CosDegrees(deltaAngle * i), 0.5f + 0.5f * SinDegrees(deltaAngle * i));
		Vec2 topUV2 = Vec2(0.5f + 0.5f * CosDegrees(deltaAngle * next), 0.5f + 0.5f * SinDegrees(deltaAngle * next));

		verts.push_back(Vertex_PCU(topCenter, color, topCenterUV));
		verts.push_back(Vertex_PCU(topVerts[i], color, topUV1));
		verts.push_back(Vertex_PCU(topVerts[next], color, topUV2));
	}
}


void AddVertsForCylinderZ3D(std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, int numSlices, Rgba8 const& tint, AABB2 const& UVs)
{
	Vec3 topCenter = Vec3(centerXY.x, centerXY.y, minMaxZ.m_max);
	Vec3 botCenter = Vec3(centerXY.x, centerXY.y, minMaxZ.m_min);

	AddVertsForCylinder3D(verts, botCenter, topCenter, radius, tint, UVs, numSlices);
}

void AddVertsForCylinderZ3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices,
	Vec2 const& centerXY, FloatRange const& minMaxZ, float radius,
	int numSlices, Rgba8 const& tint, AABB2 const& UVs)
{
	float uvWidth = UVs.m_maxs.x - UVs.m_mins.x;
	float uvHeight = UVs.m_maxs.y - UVs.m_mins.y;

	int initialVertCount = (int)verts.size();

	for (int slice = 0; slice <= numSlices; slice++) 
	{
		float angle = 2.0f * PI * static_cast<float>(slice) / static_cast<float>(numSlices);
		float cosA = cosf(angle);
		float sinA = sinf(angle);

		Vec3 normal(cosA, sinA, 0.0f);
		Vec3 tangent(-sinA, cosA, 0.0f);
		Vec3 bitangent = CrossProduct3D(tangent, normal);

		Vec3 bottomPos(centerXY.x + radius * cosA, centerXY.y + radius * sinA, minMaxZ.m_min);
		Vec2 bottomUV(UVs.m_mins.x + (static_cast<float>(slice) / numSlices) * uvWidth, UVs.m_mins.y);
		verts.push_back(Vertex_PCUTBN(bottomPos, tint, bottomUV, tangent, bitangent, normal));

		Vec3 topPos(centerXY.x + radius * cosA, centerXY.y + radius * sinA, minMaxZ.m_max);
		Vec2 topUV(UVs.m_mins.x + (static_cast<float>(slice) / numSlices) * uvWidth, UVs.m_maxs.y);
		verts.push_back(Vertex_PCUTBN(topPos, tint, topUV, tangent, bitangent, normal));
	}

	for (int slice = 0; slice < numSlices; slice++) 
	{
		int baseIdx = initialVertCount + slice * 2;
		indices.push_back(baseIdx);      // BL
		indices.push_back(baseIdx + 2); // BR
		indices.push_back(baseIdx + 1); // TL
		indices.push_back(baseIdx + 2);  // BR
		indices.push_back(baseIdx + 3);  // TR
		indices.push_back(baseIdx + 1);  // TL
	}

	if (uvHeight > 0.0f) 
	{
		int bottomCenterIdx = (int)verts.size();
		verts.push_back(Vertex_PCUTBN(
			Vec3(centerXY.x, centerXY.y, minMaxZ.m_min), tint,
			Vec2(UVs.m_mins.x + 0.5f * uvWidth, UVs.m_mins.y + 0.5f * uvHeight),
			Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)));

		int topCenterIdx = (int)verts.size();
		verts.push_back(Vertex_PCUTBN(
			Vec3(centerXY.x, centerXY.y, minMaxZ.m_max), tint,
			Vec2(UVs.m_mins.x + 0.5f * uvWidth, UVs.m_maxs.y - 0.5f * uvHeight),
			Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)));

		for (int slice = 0; slice <= numSlices; slice++) {
			float angle = 2.0f * PI * static_cast<float>(slice) / static_cast<float>(numSlices);
			float cosA = cosf(angle);
			float sinA = sinf(angle);

			Vec3 bottomPos(centerXY.x + radius * cosA, centerXY.y + radius * sinA, minMaxZ.m_min);
			Vec2 bottomUV(
				UVs.m_mins.x + 0.5f * (1.0f + cosA * 0.99f) * uvWidth,
				UVs.m_mins.y + 0.5f * (1.0f + sinA * 0.99f) * uvHeight
			);
			verts.push_back(Vertex_PCUTBN(bottomPos, tint, bottomUV,
				Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)));

			Vec3 topPos(centerXY.x + radius * cosA, centerXY.y + radius * sinA, minMaxZ.m_max);
			Vec2 topUV(
				UVs.m_mins.x + 0.5f * (1.0f + cosA * 0.99f) * uvWidth,
				UVs.m_maxs.y - 0.5f * (1.0f + sinA * 0.99f) * uvHeight
			);
			verts.push_back(Vertex_PCUTBN(topPos, tint, topUV,
				Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)));
		}

		for (int slice = 0; slice < numSlices; slice++) 
		{
			indices.push_back(bottomCenterIdx);
			indices.push_back(bottomCenterIdx + 1 + slice);
			indices.push_back(bottomCenterIdx + 2 + slice);

			indices.push_back(topCenterIdx);
			indices.push_back(topCenterIdx + 2 + slice);
			indices.push_back(topCenterIdx + 1 + slice);
		}
	}
}






void AddVertsForCylinderZWireframe3D(std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, int numSlices, float lineThickness, Rgba8 const& tint)
{
	float deltaAngle = 360.f / numSlices;
	std::vector<Vec3> bottomVerts, topVerts;

	for (int i = 0; i < numSlices; ++i)
	{
		float angle = deltaAngle * i;
		float cosA = CosDegrees(angle);
		float sinA = SinDegrees(angle);

		Vec3 bottomPoint = Vec3(centerXY.x + radius * cosA, centerXY.y + radius * sinA, minMaxZ.m_min);
		Vec3 topPoint = Vec3(centerXY.x + radius * cosA, centerXY.y + radius * sinA, minMaxZ.m_max);

		bottomVerts.push_back(bottomPoint);
		topVerts.push_back(topPoint);
	}

	for (int i = 0; i < numSlices; ++i)
	{
		int next = (i + 1) % (int)numSlices;

		AddVertsForLineSegment3D(verts, bottomVerts[i], bottomVerts[next], lineThickness, tint);
		AddVertsForLineSegment3D(verts, topVerts[i], topVerts[next], lineThickness, tint);
	}

	for (int i = 0; i < numSlices; ++i)
	{
		AddVertsForLineSegment3D(verts, bottomVerts[i], topVerts[i], lineThickness, tint);
	}
}

void AddVertsForOBB3D(std::vector<Vertex_PCU>& verts, Vec3 const& i, Vec3 const& j, Vec3 const& k, Vec3 const& halfDimensions, Vec3 const& center, const Rgba8& color, const AABB2& UVs)
{

	// Calculate local axes scaled by half dimensions
	Vec3 localX = i * halfDimensions.x;
	Vec3 localY = j * halfDimensions.y;
	Vec3 localZ = k * halfDimensions.z;

	// Define all 8 corners of the OBB (matching AABB naming)
	Vec3 frontBottomLeft = center - localX - localY - localZ;
	Vec3 frontBottomRight = center + localX - localY - localZ;
	Vec3 frontTopRight = center + localX - localY + localZ;
	Vec3 frontTopLeft = center - localX - localY + localZ;
	Vec3 backBottomLeft = center - localX + localY - localZ;
	Vec3 backBottomRight = center + localX + localY - localZ;
	Vec3 backTopRight = center + localX + localY + localZ;
	Vec3 backTopLeft = center - localX + localY + localZ;

	// Add quads for each face (order matches AABB implementation)
	// +X face (right)
	AddVertsForQuad3D(verts,
		frontBottomLeft,   // BL
		frontBottomRight,  // BR
		frontTopRight,     // TR
		frontTopLeft,      // TL
		color, UVs);

	// -X face (left)
	AddVertsForQuad3D(verts,
		backBottomRight,   // BL
		backBottomLeft,    // BR
		backTopLeft,       // TR
		backTopRight,      // TL
		color, UVs);

	// +Y face (back)
	AddVertsForQuad3D(verts,
		backBottomLeft,    // BL
		frontBottomLeft,   // BR
		frontTopLeft,     // TR
		backTopLeft,       // TL
		color, UVs);

	// -Y face (front)
	AddVertsForQuad3D(verts,
		frontBottomRight,  // BL
		backBottomRight,   // BR
		backTopRight,      // TR
		frontTopRight,     // TL
		color, UVs);

	// +Z face (top)
	AddVertsForQuad3D(verts,
		frontTopLeft,     // BL
		frontTopRight,    // BR
		backTopRight,     // TR
		backTopLeft,      // TL
		color, UVs);

	// -Z face (bottom)
	AddVertsForQuad3D(verts,
		backBottomLeft,   // BL
		backBottomRight,  // BR
		frontBottomRight, // TR
		frontBottomLeft,  // TL
		color, UVs);
}

void AddVertsForOBB3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices, OBB3 obb, const Rgba8& color, const AABB2& UVs)
{
	Vec3 i = obb.m_iBasis;
	Vec3 j = obb.m_jBasis;
	Vec3 k = obb.m_kBasis;
	Vec3 half = obb.m_halfDimensions;
	Vec3 center = obb.m_center;

	// Calculate local axes scaled by half dimensions
	Vec3 localX = i * half.x;
	Vec3 localY = j * half.y;
	Vec3 localZ = k * half.z;

	// Define all 8 corners of the OBB (matching AABB naming)
	Vec3 frontBottomLeft = center - localX - localY - localZ;
	Vec3 frontBottomRight = center + localX - localY - localZ;
	Vec3 frontTopRight = center + localX - localY + localZ;
	Vec3 frontTopLeft = center - localX - localY + localZ;
	Vec3 backBottomLeft = center - localX + localY - localZ;
	Vec3 backBottomRight = center + localX + localY - localZ;
	Vec3 backTopRight = center + localX + localY + localZ;
	Vec3 backTopLeft = center - localX + localY + localZ;

	// Add quads for each face (order matches AABB implementation)
	// +X face (right)
	AddVertsForQuad3D(verts, indices,
		frontBottomLeft,   // BL
		frontBottomRight,  // BR
		frontTopRight,     // TR
		frontTopLeft,      // TL
		color, UVs);

	// -X face (left)
	AddVertsForQuad3D(verts, indices,
		backBottomRight,   // BL
		backBottomLeft,    // BR
		backTopLeft,       // TR
		backTopRight,      // TL
		color, UVs);

	// +Y face (back)
	AddVertsForQuad3D(verts, indices,
		backBottomLeft,    // BL
		frontBottomLeft,   // BR
		frontTopLeft,     // TR
		backTopLeft,       // TL
		color, UVs);

	// -Y face (front)
	AddVertsForQuad3D(verts, indices,
		frontBottomRight,  // BL
		backBottomRight,   // BR
		backTopRight,      // TR
		frontTopRight,     // TL
		color, UVs);

	// +Z face (top)
	AddVertsForQuad3D(verts, indices,
		frontTopLeft,     // BL
		frontTopRight,    // BR
		backTopRight,     // TR
		backTopLeft,      // TL
		color, UVs);

	// -Z face (bottom)
	AddVertsForQuad3D(verts, indices,
		backBottomLeft,   // BL
		backBottomRight,  // BR
		frontBottomRight, // TR
		frontBottomLeft,  // TL
		color, UVs);
}



void AddVertsForWireframeOBB3D(std::vector<Vertex_PCU>& verts,
	Vec3 const& i, Vec3 const& j, Vec3 const& k,
	Vec3 const& halfDimensions, Vec3 const& center,
	float lineThickness, const Rgba8& color, const AABB2& UVs)
{
	UNUSED(UVs);
	Vec3 localX = i * halfDimensions.x;
	Vec3 localY = j * halfDimensions.y;
	Vec3 localZ = k * halfDimensions.z;

	Vec3 corners[8];
	corners[0] = center + localX + localY + localZ;
	corners[1] = center - localX + localY + localZ;
	corners[2] = center - localX - localY + localZ;
	corners[3] = center + localX - localY + localZ;

	corners[4] = center + localX + localY - localZ;
	corners[5] = center - localX + localY - localZ;
	corners[6] = center - localX - localY - localZ;
	corners[7] = center + localX - localY - localZ;

	AddVertsForLineSegment3D(verts, corners[0], corners[1], lineThickness, color);
	AddVertsForLineSegment3D(verts, corners[1], corners[2], lineThickness, color);
	AddVertsForLineSegment3D(verts, corners[2], corners[3], lineThickness, color);
	AddVertsForLineSegment3D(verts, corners[3], corners[0], lineThickness, color);

	AddVertsForLineSegment3D(verts, corners[4], corners[5], lineThickness, color);
	AddVertsForLineSegment3D(verts, corners[5], corners[6], lineThickness, color);
	AddVertsForLineSegment3D(verts, corners[6], corners[7], lineThickness, color);
	AddVertsForLineSegment3D(verts, corners[7], corners[4], lineThickness, color);

	AddVertsForLineSegment3D(verts, corners[0], corners[4], lineThickness, color);
	AddVertsForLineSegment3D(verts, corners[1], corners[5], lineThickness, color);
	AddVertsForLineSegment3D(verts, corners[2], corners[6], lineThickness, color);
	AddVertsForLineSegment3D(verts, corners[3], corners[7], lineThickness, color);
}




void AddVertsForPlane3D(std::vector<Vertex_PCU>& verts, Vec3 const& normal, float distFromOrigin, const Rgba8& color, const AABB2& UVs)
{
	UNUSED(color);
	UNUSED(UVs);
	Vec3 center = normal * distFromOrigin;

	Vec3 i;
	if (fabsf(normal.z) < 0.999f)
		i = CrossProduct3D(normal, Vec3(0, 0, 1)).GetNormalized();
	else
		i = CrossProduct3D(normal, Vec3(0, 1, 0)).GetNormalized();

	Vec3 j = CrossProduct3D(normal, i).GetNormalized();

	float halfSize = 25.f;
	Vec3 halfI = i * halfSize;
	Vec3 halfJ = j * halfSize;

	float spacing = 1.f;        
	float lineHalfThickness = 0.02f;
	int numLines = static_cast<int>((halfSize * 2.f) / spacing);

	for (int n = -numLines / 2; n <= numLines / 2; ++n)
	{
		float offset = spacing * n;

		Vec3 startJ = center + (j * offset) - halfI;
		Vec3 endJ = center + (j * offset) + halfI;
		AddVertsForLineSegment3D(verts, startJ, endJ, lineHalfThickness * 2.f, Rgba8(255, 0, 0, 100));

		Vec3 startI = center + (i * offset) - halfJ;
		Vec3 endI = center + (i * offset) + halfJ;
		AddVertsForLineSegment3D(verts, startI, endI, lineHalfThickness * 2.f, Rgba8(0, 255, 0, 100));
	}
}


void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8& color, const AABB2& UVs, int numSlices)
{
	Vec3 axis = (end - start).GetNormalized();
	Vec3 up = Vec3(0.f, 0.f, 1.f);
	if (fabsf(DotProduct3D(axis, up)) > 0.999f)
	{
		up = Vec3(0.f, 1.f, 0.f);
	}
	Vec3 right = CrossProduct3D(up, axis).GetNormalized();
	Vec3 forward = CrossProduct3D(axis, right).GetNormalized();

	float deltaAngle = 360.f / numSlices;
	float uStep = 1.0f / numSlices;

	std::vector<Vec3> bottomVerts;
	std::vector<Vec2> bottomUVs;

	for (int i = 0; i < numSlices; ++i)
	{
		float angle = deltaAngle * i;
		float cosA = CosDegrees(angle);
		float sinA = SinDegrees(angle);

		Vec3 offset = (right * cosA + forward * sinA) * radius;
		bottomVerts.push_back(start + offset);

		float u = i * uStep;
		bottomUVs.push_back(Vec2(UVs.m_mins.x + u * UVs.GetDimensions().x, UVs.m_mins.y));
	}

	Vec3 bottomCenter = start;
	Vec2 bottomCenterUV = Vec2((UVs.m_mins.x + UVs.m_maxs.x) * 0.5f, UVs.m_mins.y);
	for (int i = 0; i < numSlices; ++i)
	{
		int next = (i + 1) % numSlices;
		verts.push_back(Vertex_PCU(bottomCenter, color, bottomCenterUV));
		verts.push_back(Vertex_PCU(bottomVerts[next], color, bottomUVs[next]));
		verts.push_back(Vertex_PCU(bottomVerts[i], color, bottomUVs[i]));
	}

	for (int i = 0; i < numSlices; ++i)
	{
		int next = (i + 1) % numSlices;
		verts.push_back(Vertex_PCU(bottomVerts[i], color, bottomUVs[i]));
		verts.push_back(Vertex_PCU(bottomVerts[next], color, bottomUVs[next]));
		verts.push_back(Vertex_PCU(end, color, Vec2((UVs.m_mins.x + UVs.m_maxs.x) * 0.5f, UVs.m_maxs.y)));
	}
}

void AddVertsForPyramidArrow3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8& color)
{
	Vec3 direction = end - start;
	direction = direction.GetNormalized();

	float pyramidHeight = radius * 8.0f;

	Vec3 cylinderEnd = end - direction * pyramidHeight;

	AABB2 UVs = AABB2::ZERO_TO_ONE;
	AddVertsForCylinder3D(verts, start, cylinderEnd, radius, color, UVs, 16);

	Vec3 pyramidBaseCenter = cylinderEnd;
	Vec3 pyramidTop = end;

	Vec3 right, up;

	float pyramidSize = 2.f;

	if (fabsf(direction.z) > 0.999f)
	{
		right = CrossProduct3D(direction, Vec3(0, 1, 0)).GetNormalized() * (radius * pyramidSize);
		up = CrossProduct3D(right, direction).GetNormalized() * (radius * pyramidSize);
	}
	else
	{
		right = CrossProduct3D(direction, Vec3(0, 0, 1)).GetNormalized() * (radius * pyramidSize);
		up = CrossProduct3D(right, direction).GetNormalized() * (radius * pyramidSize);
	}

	Vec3 base1 = pyramidBaseCenter - right - up;
	Vec3 base2 = pyramidBaseCenter + right - up;
	Vec3 base3 = pyramidBaseCenter + right + up;
	Vec3 base4 = pyramidBaseCenter - right + up;

	Rgba8 darkColor = color;
	darkColor.r = static_cast<unsigned char>(color.r * 0.5f);
	darkColor.g = static_cast<unsigned char>(color.g * 0.5f);
	darkColor.b = static_cast<unsigned char>(color.b * 0.5f);

	verts.push_back(Vertex_PCU{ base2, color });
	verts.push_back(Vertex_PCU{ base1, color });
	verts.push_back(Vertex_PCU{ pyramidTop, color });

	verts.push_back(Vertex_PCU{ base1, darkColor });
	verts.push_back(Vertex_PCU{ base4, darkColor });
	verts.push_back(Vertex_PCU{ pyramidTop, darkColor });

	verts.push_back(Vertex_PCU{ base4, color });
	verts.push_back(Vertex_PCU{ base3, color });
	verts.push_back(Vertex_PCU{ pyramidTop, color });

	verts.push_back(Vertex_PCU{ base3, darkColor });
	verts.push_back(Vertex_PCU{ base2, darkColor });
	verts.push_back(Vertex_PCU{ pyramidTop, darkColor });

	verts.push_back(Vertex_PCU{ base1, darkColor });
	verts.push_back(Vertex_PCU{ base2, darkColor });
	verts.push_back(Vertex_PCU{ base3, darkColor });

	verts.push_back(Vertex_PCU{ base1, darkColor });
	verts.push_back(Vertex_PCU{ base3, darkColor });
	verts.push_back(Vertex_PCU{ base4, darkColor });
}




void AddVertsForDisc2D(std::vector<Vertex_PCU>& verts, Vec2 const& discCenter, float discRadius, Rgba8 const& color)
{
	const int numSides = 32;
	const float deltaAngle = 360.0f / numSides;

	Vec2 uvCenter = Vec2(0.5f, 0.5f);

	Vec2 uvPrev = Vec2(1.f, 0.5f);
	Vec2 prevPoint = Vec2(discRadius, 0.f) + discCenter;

	for (int sideIndex = 1; sideIndex <= numSides; ++sideIndex)
	{
		float angleDegrees = deltaAngle * sideIndex;
		Vec2 dir = Vec2::MakeFromPolarDegrees(angleDegrees);
		Vec2 point = dir * discRadius + discCenter;
		Vec2 uvPoint = dir * 0.5f + uvCenter;

		verts.push_back(Vertex_PCU(Vec3(discCenter.x, discCenter.y, 0.f), color, uvCenter));
		verts.push_back(Vertex_PCU(Vec3(prevPoint.x, prevPoint.y, 0.f), color, uvPrev));
		verts.push_back(Vertex_PCU(Vec3(point.x, point.y, 0.f), color, uvPoint));

		prevPoint = point;
		uvPrev = uvPoint;
	}
}

void AddVertsForDisc2D(std::vector<Vertex_PCU>& verts, Disc2 const& disc, Rgba8 const& color)
{
	AddVertsForDisc2D(verts, disc.m_center, disc.m_radius, color);
}

void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, float thickness, Rgba8 const& color)
{
	const int NUM_SIDES = 32;
	const float DEGREES_PER_SIDE = 360.0f / NUM_SIDES;

	float innerRadius = radius - thickness * 0.5f;
	float outerRadius = radius + thickness * 0.5f;

	for (int sideIndex = 0; sideIndex < NUM_SIDES; sideIndex++)
	{
		float startDegrees = DEGREES_PER_SIDE * sideIndex;
		float endDegrees = DEGREES_PER_SIDE * (sideIndex + 1);

		float startRadians = ConvertDegreesToRadians(startDegrees);
		float endRadians = ConvertDegreesToRadians(endDegrees);

		Vec2 innerStart = center + Vec2(cosf(startRadians), sinf(startRadians)) * innerRadius;
		Vec2 outerStart = center + Vec2(cosf(startRadians), sinf(startRadians)) * outerRadius;
		Vec2 innerEnd = center + Vec2(cosf(endRadians), sinf(endRadians)) * innerRadius;
		Vec2 outerEnd = center + Vec2(cosf(endRadians), sinf(endRadians)) * outerRadius;

		verts.push_back(Vertex_PCU(outerStart, color, Vec2(0, 0)));
		verts.push_back(Vertex_PCU(outerEnd, color, Vec2(0, 0)));
		verts.push_back(Vertex_PCU(innerEnd, color, Vec2(0, 0)));

		verts.push_back(Vertex_PCU(outerStart, color, Vec2(0, 0)));
		verts.push_back(Vertex_PCU(innerEnd, color, Vec2(0, 0)));
		verts.push_back(Vertex_PCU(innerStart, color, Vec2(0, 0)));
	}
}


void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& alignedBox, Rgba8 const& color)
{
	Vec3 BL = Vec3(alignedBox.m_mins.x, alignedBox.m_mins.y, 0);
	Vec3 BR = Vec3(alignedBox.m_maxs.x, alignedBox.m_mins.y, 0);
	Vec3 TR = Vec3(alignedBox.m_maxs.x, alignedBox.m_maxs.y, 0);
	Vec3 TL = Vec3(alignedBox.m_mins.x, alignedBox.m_maxs.y, 0);

	verts.push_back(Vertex_PCU(BL, color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(BR, color, Vec2(1.f, 0.f)));
	verts.push_back(Vertex_PCU(TR, color, Vec2(1.f, 1.f)));

	verts.push_back(Vertex_PCU(BL, color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(TR, color, Vec2(1.f, 1.f)));
	verts.push_back(Vertex_PCU(TL, color, Vec2(0.f, 1.f)));
}

void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& alignedBox, Rgba8 const& color, Vec2 const& uvMins, Vec2 const& uvMaxs)
{
	Vec3 BL = Vec3(alignedBox.m_mins.x, alignedBox.m_mins.y, 0);
	Vec3 BR = Vec3(alignedBox.m_maxs.x, alignedBox.m_mins.y, 0);
	Vec3 TR = Vec3(alignedBox.m_maxs.x, alignedBox.m_maxs.y, 0);
	Vec3 TL = Vec3(alignedBox.m_mins.x, alignedBox.m_maxs.y, 0);

	verts.push_back(Vertex_PCU(BL, color, uvMins));
	verts.push_back(Vertex_PCU(BR, color, Vec2(uvMaxs.x, uvMins.y)));
	verts.push_back(Vertex_PCU(TR, color, uvMaxs));

	verts.push_back(Vertex_PCU(BL, color, uvMins));
	verts.push_back(Vertex_PCU(TR, color, uvMaxs));
	verts.push_back(Vertex_PCU(TL, color, Vec2(uvMins.x, uvMaxs.y)));
}

void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, const Rgba8& color, const AABB2& UVs)
{
	// Extract min and max corners of the AABB
	Vec3 mins = bounds.m_mins;
	Vec3 maxs = bounds.m_maxs;

	// Define all 8 corners of the AABB
	Vec3 backBottomLeft(mins.x, maxs.y, mins.z);   // Back-left-bottom
	Vec3 backBottomRight(maxs.x, maxs.y, mins.z);  // Back-right-bottom
	Vec3 backTopRight(maxs.x, maxs.y, maxs.z);	   // Back-right-top
	Vec3 backTopLeft(mins.x, maxs.y, maxs.z);	   // Back-left-top

	Vec3 frontBottomLeft(mins.x, mins.y, mins.z);  // Front-left-bottom
	Vec3 frontBottomRight(maxs.x, mins.y, mins.z); // Front-right-bottom
	Vec3 frontTopRight(maxs.x, mins.y, maxs.z);	   // Front-right-top	
	Vec3 frontTopLeft(mins.x, mins.y, maxs.z);	   // Front-left-top	

	// Front face (+x)
	AddVertsForQuad3D(verts, backBottomLeft, backBottomRight, backTopRight, backTopLeft, color, UVs);

	// Back face (-x)
	AddVertsForQuad3D(verts, frontBottomRight, frontBottomLeft, frontTopLeft, frontTopRight, color, UVs);

	// Left face (+y)
	AddVertsForQuad3D(verts, frontBottomLeft, backBottomLeft, backTopLeft, frontTopLeft, color, UVs);

	// Right face (-y)
	AddVertsForQuad3D(verts, backBottomRight, frontBottomRight, frontTopRight, backTopRight, color, UVs);

	// Top face (+z)
	AddVertsForQuad3D(verts, backTopLeft, backTopRight, frontTopRight, frontTopLeft, color, UVs);

	// Bottom face (-z)
	AddVertsForQuad3D(verts, frontBottomLeft, frontBottomRight, backBottomRight, backBottomLeft, color, UVs);
}

void AddVertsForAABB3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes,
	const AABB3& bounds, const Rgba8& color, const AABB2& UVs)
{
	// Extract min and max corners of the AABB
	Vec3 mins = bounds.m_mins;
	Vec3 maxs = bounds.m_maxs;

	// Define all 8 corners of the AABB
	Vec3 frontBottomLeft(mins.x, mins.y, mins.z);  // Front-left-bottom
	Vec3 frontBottomRight(maxs.x, mins.y, mins.z); // Front-right-bottom
	Vec3 frontTopRight(maxs.x, mins.y, maxs.z);    // Front-right-top
	Vec3 frontTopLeft(mins.x, mins.y, maxs.z);     // Front-left-top

	Vec3 backBottomLeft(mins.x, maxs.y, mins.z);   // Back-left-bottom
	Vec3 backBottomRight(maxs.x, maxs.y, mins.z);  // Back-right-bottom
	Vec3 backTopRight(maxs.x, maxs.y, maxs.z);     // Back-right-top
	Vec3 backTopLeft(mins.x, maxs.y, maxs.z);      // Back-left-top

	// (+X)
	AddVertsForQuad3D(verts, indexes,
		frontBottomLeft,  // BL
		frontBottomRight, // BR
		frontTopRight,    // TR
		frontTopLeft,     // TL
		color, UVs);

	// (-X)
	AddVertsForQuad3D(verts, indexes,
		backBottomRight,  // BL
		backBottomLeft,  // BR
		backTopLeft,      // TR
		backTopRight,    // TL
		color, UVs);

	// (+Y)
	AddVertsForQuad3D(verts, indexes,
		backBottomLeft,  // BL
		frontBottomLeft,  // BR
		frontTopLeft,    // TR
		backTopLeft,      // TL
		color, UVs);

	// (-Y)
	AddVertsForQuad3D(verts, indexes,
		frontBottomRight, // BL
		backBottomRight,  // BR
		backTopRight,     // TR
		frontTopRight,    // TL
		color, UVs);

	// (+Z)
	AddVertsForQuad3D(verts, indexes,
		frontTopLeft,     // BL
		frontTopRight,    // BR
		backTopRight,     // TR
		backTopLeft,      // TL
		color, UVs);

	// (-Z)
	AddVertsForQuad3D(verts, indexes,
		backBottomLeft,   // BL
		backBottomRight,  // BR
		frontBottomRight, // TR
		frontBottomLeft,  // TL
		color, UVs);
}



void AddVertsForAABB3DWall(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const AABB3& bounds, const Rgba8& color, const AABB2& UVs)
{
	Vec3 FBL = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 FBR = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 FTR = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);
	Vec3 FTL = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);


	Vec3 BBL = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 BBR = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 BTR = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 BTL = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);

	AddVertsForQuad3D(verts, indexes, BBR, BTR, FTR, FBR, color, UVs);
	AddVertsForQuad3D(verts, indexes, BTL, BBL, FBL, FTL, color, UVs);

	AddVertsForQuad3D(verts, indexes, BTR, BTL, FTL, FTR, color, UVs);
	AddVertsForQuad3D(verts, indexes, BBL, BBR, FBR, FBL, color, UVs);


}

void AddVertsForAABBWireframe3D(std::vector<Vertex_PCU>& verts,
	AABB3 const& bounds, float lineThickness,
	Rgba8 const& tint)
{
	Vec3 FBL = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 FBR = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 FTR = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);
	Vec3 FTL = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);

	Vec3 BBL = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 BBR = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 BTR = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 BTL = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);

	AddVertsForLineSegment3D(verts, FBL, FBR, lineThickness, tint);
	AddVertsForLineSegment3D(verts, FBR, FTR, lineThickness, tint);
	AddVertsForLineSegment3D(verts, FTR, FTL, lineThickness, tint);
	AddVertsForLineSegment3D(verts, FTL, FBL, lineThickness, tint);

	AddVertsForLineSegment3D(verts, BBL, BBR, lineThickness, tint);
	AddVertsForLineSegment3D(verts, BBR, BTR, lineThickness, tint);
	AddVertsForLineSegment3D(verts, BTR, BTL, lineThickness, tint);
	AddVertsForLineSegment3D(verts, BTL, BBL, lineThickness, tint);

	AddVertsForLineSegment3D(verts, FBL, BBL, lineThickness, tint);
	AddVertsForLineSegment3D(verts, FBR, BBR, lineThickness, tint);
	AddVertsForLineSegment3D(verts, FTR, BTR, lineThickness, tint);
	AddVertsForLineSegment3D(verts, FTL, BTL, lineThickness, tint);
}



void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& orientedBox, Rgba8 const& color)
{
	Vec2 center = orientedBox.m_center;
	Vec2 i = orientedBox.m_iBasisNormal;
	Vec2 j = i.GetRotated90Degrees();
	Vec2 half = orientedBox.m_halfDimensions;

	Vec2 topRight = center + i * half.x + j * half.y;
	Vec2 topLeft = center - i * half.x + j * half.y;
	Vec2 bottomRight = center + i * half.x - j * half.y;
	Vec2 bottomLeft = center - i * half.x - j * half.y;

	verts.push_back(Vertex_PCU(Vec3(topRight.x, topRight.y, 0.f), color, Vec2(1.f, 1.f)));
	verts.push_back(Vertex_PCU(Vec3(topLeft.x, topLeft.y, 0.f), color, Vec2(0.f, 1.f)));
	verts.push_back(Vertex_PCU(Vec3(bottomRight.x, bottomRight.y, 0.f), color, Vec2(1.f, 0.f)));

	verts.push_back(Vertex_PCU(Vec3(bottomRight.x, bottomRight.y, 0.f), color, Vec2(1.f, 0.f)));
	verts.push_back(Vertex_PCU(Vec3(topLeft.x, topLeft.y, 0.f), color, Vec2(0.f, 1.f)));
	verts.push_back(Vertex_PCU(Vec3(bottomLeft.x, bottomLeft.y, 0.f), color, Vec2(0.f, 0.f)));
}



void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color)
{
	Vec2 direction = (boneEnd - boneStart).GetNormalized();
	Vec2 normal = direction.GetRotated90Degrees();

	Vec3 rectBL(boneStart.x - normal.x * radius, boneStart.y - normal.y * radius, 0.0f);
	Vec3 rectBR(boneStart.x + normal.x * radius, boneStart.y + normal.y * radius, 0.0f);
	Vec3 rectTL(boneEnd.x - normal.x * radius, boneEnd.y - normal.y * radius, 0.0f);
	Vec3 rectTR(boneEnd.x + normal.x * radius, boneEnd.y + normal.y * radius, 0.0f);

	verts.push_back(Vertex_PCU(rectBL, color, Vec2(0.0f, 0.0f)));
	verts.push_back(Vertex_PCU(rectTL, color, Vec2(0.0f, 1.0f)));
	verts.push_back(Vertex_PCU(rectBR, color, Vec2(1.0f, 0.0f)));

	verts.push_back(Vertex_PCU(rectBR, color, Vec2(1.0f, 0.0f)));
	verts.push_back(Vertex_PCU(rectTL, color, Vec2(0.0f, 1.0f)));
	verts.push_back(Vertex_PCU(rectTR, color, Vec2(1.0f, 1.0f)));

	const int numSides = 32;

	for (int sideIndex = 0; sideIndex < numSides; ++sideIndex)
	{
		float fraction0 = static_cast<float>(sideIndex) / numSides;
		float fraction1 = static_cast<float>(sideIndex + 1) / numSides;

		Vec2 capStart0 = boneStart + (normal * radius * cosf(fraction0 * 3.14159265f)) - (direction * radius * sinf(fraction0 * 3.14159265f));
		Vec2 capStart1 = boneStart + (normal * radius * cosf(fraction1 * 3.14159265f)) - (direction * radius * sinf(fraction1 * 3.14159265f));

		verts.push_back(Vertex_PCU(Vec3(boneStart.x, boneStart.y, 0.f), color, Vec2(0.5f, 0.5f)));
		verts.push_back(Vertex_PCU(Vec3(capStart0.x, capStart0.y, 0.f), color, Vec2(0.5f, 0.5f)));
		verts.push_back(Vertex_PCU(Vec3(capStart1.x, capStart1.y, 0.f), color, Vec2(0.5f, 0.5f)));

		Vec2 capEnd0 = boneEnd + (normal * radius * cosf(fraction0 * 3.14159265f)) + (direction * radius * sinf(fraction0 * 3.14159265f));
		Vec2 capEnd1 = boneEnd + (normal * radius * cosf(fraction1 * 3.14159265f)) + (direction * radius * sinf(fraction1 * 3.14159265f));

		verts.push_back(Vertex_PCU(Vec3(boneEnd.x, boneEnd.y, 0.f), color, Vec2(0.5f, 0.5f)));
		verts.push_back(Vertex_PCU(Vec3(capEnd1.x, capEnd1.y, 0.f), color, Vec2(0.5f, 0.5f)));
		verts.push_back(Vertex_PCU(Vec3(capEnd0.x, capEnd0.y, 0.f), color, Vec2(0.5f, 0.5f)));
	}
}

void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Capsule2 const& capsule, Rgba8 const& color)
{
	AddVertsForCapsule2D(verts, capsule.m_start, capsule.m_end, capsule.m_radius, color);
}

void AddVertsForTriangle2D(std::vector<Vertex_PCU>& verts, Vec2 const& ccw0, Vec2 const& ccw1, Vec2 const& ccw2, Rgba8 const& color)
{
	verts.push_back(Vertex_PCU(Vec3(ccw0.x, ccw0.y, 0.0f), color, Vec2(0.0f, 0.0f)));
	verts.push_back(Vertex_PCU(Vec3(ccw1.x, ccw1.y, 0.0f), color, Vec2(0.0f, 1.0f)));
	verts.push_back(Vertex_PCU(Vec3(ccw2.x, ccw2.y, 0.0f), color, Vec2(1.0f, 1.0f)));
}

void AddVertsForTriangle2D(std::vector<Vertex_PCU>& verts, Triangle2 const& triangle, Rgba8 const& color)
{
	AddVertsForTriangle2D(verts, triangle.m_pointsCounterClockwise[0], triangle.m_pointsCounterClockwise[1], triangle.m_pointsCounterClockwise[2], color);
}

void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color)
{
	Vec2 direction = end - start;
	float length = direction.GetLength();

	if (length > 0.0f) {
		direction.Normalize();
	}

	Vec2 normal = direction.GetRotated90Degrees() * (thickness * 0.5f);

	Vec3 v0 = Vec3(start.x - normal.x, start.y - normal.y, 0.0f);
	Vec3 v1 = Vec3(start.x + normal.x, start.y + normal.y, 0.0f);
	Vec3 v2 = Vec3(end.x - normal.x, end.y - normal.y, 0.0f);
	Vec3 v3 = Vec3(end.x + normal.x, end.y + normal.y, 0.0f);

	verts.push_back(Vertex_PCU(v0, color, Vec2(0.0f, 0.0f)));
	verts.push_back(Vertex_PCU(v2, color, Vec2(1.0f, 0.0f)));
	verts.push_back(Vertex_PCU(v1, color, Vec2(0.0f, 1.0f)));

	verts.push_back(Vertex_PCU(v1, color, Vec2(1.0f, 0.0f)));
	verts.push_back(Vertex_PCU(v2, color, Vec2(0.0f, 1.0f)));
	verts.push_back(Vertex_PCU(v3, color, Vec2(1.0f, 1.0f)));
}

void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, LineSegment2 const& lineSeg, float thickness, Rgba8 const& color)
{
	AddVertsForLineSegment2D(verts, lineSeg.m_start, lineSeg.m_end, thickness, color);
}

void AddVertsForLineSegment3D(std::vector<Vertex_PCU>& verts,
	Vec3 const& start, Vec3 const& end,
	float thickness, Rgba8 const& color)
{
	Vec3 direction = (end - start).GetNormalized();

	Vec3 offset = direction * (thickness * 0.51f);
	Vec3 newStart = start - offset;
	Vec3 newEnd = end + offset;

	Vec3 up = (fabsf(direction.y) < 0.99f) ? Vec3(0, 1, 0) : Vec3(1, 0, 0);

	Vec3 right = CrossProduct3D(direction, up).GetNormalized();
	Vec3 forward = CrossProduct3D(right, direction).GetNormalized();

	float halfThickness = thickness * 0.5f;

	Vec3 bottomBackLeft = newStart - right * halfThickness - forward * halfThickness;
	Vec3 bottomBackRight = newStart + right * halfThickness - forward * halfThickness;
	Vec3 bottomFrontLeft = newStart - right * halfThickness + forward * halfThickness;
	Vec3 bottomFrontRight = newStart + right * halfThickness + forward * halfThickness;

	Vec3 topBackLeft = newEnd - right * halfThickness - forward * halfThickness;
	Vec3 topBackRight = newEnd + right * halfThickness - forward * halfThickness;
	Vec3 topFrontLeft = newEnd - right * halfThickness + forward * halfThickness;
	Vec3 topFrontRight = newEnd + right * halfThickness + forward * halfThickness;

	// Bottom face
	verts.push_back(Vertex_PCU(bottomBackLeft, color));
	verts.push_back(Vertex_PCU(bottomBackRight, color));
	verts.push_back(Vertex_PCU(bottomFrontLeft, color));

	verts.push_back(Vertex_PCU(bottomBackRight, color));
	verts.push_back(Vertex_PCU(bottomFrontRight, color));
	verts.push_back(Vertex_PCU(bottomFrontLeft, color));

	// Top face
	verts.push_back(Vertex_PCU(topBackLeft, color));
	verts.push_back(Vertex_PCU(topFrontLeft, color));
	verts.push_back(Vertex_PCU(topBackRight, color));

	verts.push_back(Vertex_PCU(topBackRight, color));
	verts.push_back(Vertex_PCU(topFrontLeft, color));
	verts.push_back(Vertex_PCU(topFrontRight, color));

	// Front face
	verts.push_back(Vertex_PCU(bottomFrontLeft, color));
	verts.push_back(Vertex_PCU(bottomFrontRight, color));
	verts.push_back(Vertex_PCU(topFrontLeft, color));

	verts.push_back(Vertex_PCU(bottomFrontRight, color));
	verts.push_back(Vertex_PCU(topFrontRight, color));
	verts.push_back(Vertex_PCU(topFrontLeft, color));

	// Back face
	verts.push_back(Vertex_PCU(bottomBackLeft, color));
	verts.push_back(Vertex_PCU(topBackLeft, color));
	verts.push_back(Vertex_PCU(bottomBackRight, color));

	verts.push_back(Vertex_PCU(bottomBackRight, color));
	verts.push_back(Vertex_PCU(topBackLeft, color));
	verts.push_back(Vertex_PCU(topBackRight, color));

	// Left face
	verts.push_back(Vertex_PCU(bottomBackLeft, color));
	verts.push_back(Vertex_PCU(bottomFrontLeft, color));
	verts.push_back(Vertex_PCU(topBackLeft, color));

	verts.push_back(Vertex_PCU(bottomFrontLeft, color));
	verts.push_back(Vertex_PCU(topFrontLeft, color));
	verts.push_back(Vertex_PCU(topBackLeft, color));

	// Right face
	verts.push_back(Vertex_PCU(bottomBackRight, color));
	verts.push_back(Vertex_PCU(topBackRight, color));
	verts.push_back(Vertex_PCU(bottomFrontRight, color));

	verts.push_back(Vertex_PCU(bottomFrontRight, color));
	verts.push_back(Vertex_PCU(topBackRight, color));
	verts.push_back(Vertex_PCU(topFrontRight, color));
}


void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 const& color)
{
	AddVertsForLineSegment2D(verts, tailPos, tipPos, lineThickness, color);

	Vec2 direction = (tipPos - tailPos).GetNormalized();

	Vec2 arrowLeftDir = direction.GetRotatedDegrees(-135.f); 
	Vec2 arrowRightDir = direction.GetRotatedDegrees(135.f);

	Vec2 arrowLeft = tipPos + arrowLeftDir * arrowSize;
	Vec2 arrowRight = tipPos + arrowRightDir * arrowSize;

	AddVertsForLineSegment2D(verts, tipPos, arrowLeft, lineThickness, color);
	AddVertsForLineSegment2D(verts, tipPos, arrowRight, lineThickness, color);
}

void AddVertsForQuad2D(std::vector<Vertex_PCU>& verts, Vec2 ccw0, Vec2 ccw1, Vec2 ccw2, Vec2 ccw3, Rgba8 tint, Vec2 uv0, Vec2 uv1, Vec2 uv2, Vec2 uv3)
{
	verts.push_back(Vertex_PCU(Vec3(ccw0.x, ccw0.y, 0.f), tint, uv0));
	verts.push_back(Vertex_PCU(Vec3(ccw1.x, ccw1.y, 0.f), tint, uv1));
	verts.push_back(Vertex_PCU(Vec3(ccw2.x, ccw2.y, 0.f), tint, uv2));
												 
	verts.push_back(Vertex_PCU(Vec3(ccw0.x, ccw0.y, 0.f), tint, uv0));
	verts.push_back(Vertex_PCU(Vec3(ccw2.x, ccw2.y, 0.f), tint, uv2));
	verts.push_back(Vertex_PCU(Vec3(ccw3.x, ccw3.y, 0.f), tint, uv3));
}

void AddVertsForQuad2D(std::vector<Vertex_PCU>& verts, Vec2 ccw0, Vec2 ccw1, Vec2 ccw2, Vec2 ccw3, Rgba8 tint, AABB2 const& UVs)
{
	Vec2 uv0 = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uv1 = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uv2 = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uv3 = Vec2(UVs.m_mins.x, UVs.m_maxs.y);

	verts.push_back(Vertex_PCU(Vec3(ccw0.x, ccw0.y, 0.f), tint, uv0));
	verts.push_back(Vertex_PCU(Vec3(ccw1.x, ccw1.y, 0.f), tint, uv1));
	verts.push_back(Vertex_PCU(Vec3(ccw2.x, ccw2.y, 0.f), tint, uv2));

	verts.push_back(Vertex_PCU(Vec3(ccw0.x, ccw0.y, 0.f), tint, uv0));
	verts.push_back(Vertex_PCU(Vec3(ccw2.x, ccw2.y, 0.f), tint, uv2));
	verts.push_back(Vertex_PCU(Vec3(ccw3.x, ccw3.y, 0.f), tint, uv3));
}

void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, Vec2 uv0, Vec2 uv1, Vec2 uv2, Vec2 uv3, const Rgba8& color)
{
	verts.push_back(Vertex_PCU(bottomLeft, color, uv0));
	verts.push_back(Vertex_PCU(bottomRight, color, uv1));
	verts.push_back(Vertex_PCU(topRight, color, uv2));

	verts.push_back(Vertex_PCU(bottomLeft, color, uv0));
	verts.push_back(Vertex_PCU(topRight, color, uv2));
	verts.push_back(Vertex_PCU(topLeft, color, uv3));

}



void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts,
	const Vec3& bottomLeft, const Vec3& bottomRight,
	const Vec3& topRight, const Vec3& topLeft,
	const Rgba8& color, const AABB2& UVs)
{
	Vertex_PCU v0, v1, v2, v3;
	v0.m_position = bottomLeft;
	v1.m_position = bottomRight;
	v2.m_position = topRight;
	v3.m_position = topLeft;

	v0.m_color = color;
	v1.m_color = color;
	v2.m_color = color;
	v3.m_color = color;

	v0.m_uvTexCoords = Vec2(UVs.m_mins.x, UVs.m_mins.y); 
	v1.m_uvTexCoords = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	v2.m_uvTexCoords = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	v3.m_uvTexCoords = Vec2(UVs.m_mins.x, UVs.m_maxs.y);

	verts.push_back(v0);
	verts.push_back(v1);
	verts.push_back(v2);

	verts.push_back(v0);
	verts.push_back(v2);
	verts.push_back(v3);
}

void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs)
{
	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);    // Bottom-left UV
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);    // Bottom-right UV
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);    // Top-right UV
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);    // Top-left UV

	Vec3 edgeHorz = bottomRight - bottomLeft;  // Right edge
	Vec3 edgeVert = topLeft - bottomLeft;      // Up edge
	Vec3 normal = CrossProduct3D(edgeHorz, edgeVert).GetNormalized();

	Vec3 tangent = edgeHorz.GetNormalized();
	Vec3 bitangent = CrossProduct3D(normal, tangent).GetNormalized();

	unsigned int startIndex = static_cast<unsigned int>(verts.size());

	verts.push_back(Vertex_PCUTBN(bottomLeft, color, uvBL, tangent, bitangent, normal));  // 0
	verts.push_back(Vertex_PCUTBN(bottomRight, color, uvBR, tangent, bitangent, normal));  // 1
	verts.push_back(Vertex_PCUTBN(topRight, color, uvTR, tangent, bitangent, normal));  // 2
	verts.push_back(Vertex_PCUTBN(topLeft, color, uvTL, tangent, bitangent, normal));  // 3

	indexes.push_back(startIndex + 0);
	indexes.push_back(startIndex + 1);
	indexes.push_back(startIndex + 2);

	indexes.push_back(startIndex + 0);
	indexes.push_back(startIndex + 2);
	indexes.push_back(startIndex + 3);
}


void AddVertsForRoundedQuad3D(std::vector<Vertex_PCUTBN>& vertexes, const Vec3& bottomLeft, const Vec3& bottomRight,
	const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs)
{
	Vec3 topCenter = (topLeft + topRight) / 2.f;
	Vec3 bottomCenter = (bottomLeft + bottomRight) / 2.f;

	Vec3 normal = CrossProduct3D(bottomRight - bottomLeft, topLeft - bottomLeft).GetNormalized();
	Vec3 leftNormal = (bottomLeft - bottomRight).GetNormalized();
	Vec3 rightNormal = (bottomRight - bottomLeft).GetNormalized();

	Vec3 tangent = (bottomRight - bottomLeft).GetNormalized();
	Vec3 bitangent = (topLeft - bottomLeft).GetNormalized();

	Vec2 uvBL = UVs.m_mins;
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR = UVs.m_maxs;
	Vec2 uvCT = Vec2((uvTL.x + uvTR.x) * 0.5f, uvTR.y);
	Vec2 uvCB = Vec2((uvBL.x + uvBR.x) * 0.5f, uvBL.y);

	vertexes.push_back(Vertex_PCUTBN(bottomCenter, color, uvCB, normal, bitangent, normal));
	vertexes.push_back(Vertex_PCUTBN(topLeft, color, uvTL, -tangent, bitangent, leftNormal));
	vertexes.push_back(Vertex_PCUTBN(bottomLeft, color, uvBL, -tangent, bitangent, leftNormal));

	vertexes.push_back(Vertex_PCUTBN(bottomCenter, color, uvCB, normal, bitangent, normal));
	vertexes.push_back(Vertex_PCUTBN(topCenter, color, uvCT, normal, bitangent, normal));
	vertexes.push_back(Vertex_PCUTBN(topLeft, color, uvTL, -tangent, bitangent, leftNormal));

	vertexes.push_back(Vertex_PCUTBN(bottomCenter, color, uvCB, normal, bitangent, normal));
	vertexes.push_back(Vertex_PCUTBN(bottomRight, color, uvBR, tangent, bitangent, rightNormal));
	vertexes.push_back(Vertex_PCUTBN(topCenter, color, uvCT, normal, bitangent, normal));

	vertexes.push_back(Vertex_PCUTBN(bottomRight, color, uvBR, tangent, bitangent, rightNormal));
	vertexes.push_back(Vertex_PCUTBN(topRight, color, uvTR, tangent, bitangent, rightNormal));
	vertexes.push_back(Vertex_PCUTBN(topCenter, color, uvCT, normal, bitangent, normal));
}

void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numSlices, int numStacks)
{
	float uMin = UVs.m_mins.x;
	float uMax = UVs.m_maxs.x;
	float vMin = UVs.m_mins.y;
	float vMax = UVs.m_maxs.y;

	float deltaTheta = 360.0f / static_cast<float>(numSlices);
	float deltaPhi = 180.0f / static_cast<float>(numStacks);

	std::vector<Vec3> positions;
	std::vector<Vec2> uvs;

	for (int stack = 0; stack <= numStacks; ++stack)
	{
		float phi = 180.0f - stack * deltaPhi; 
		float v = vMin + (vMax - vMin) * (static_cast<float>(stack) / static_cast<float>(numStacks));

		for (int slice = numSlices; slice >= 0; --slice)
		{
			float theta = slice * deltaTheta;
			float u = uMin + (uMax - uMin) * (static_cast<float>(slice) / static_cast<float>(numSlices));

			float x = radius * SinDegrees(phi) * CosDegrees(theta);
			float y = radius * SinDegrees(phi) * SinDegrees(theta);
			float z = radius * CosDegrees(phi);

			Vec3 position = Vec3(x, y, z) + center;
			Vec2 uv(u, v);

			positions.push_back(position);
			uvs.push_back(uv);
		}
	}

	for (int i = 0; i < numStacks; ++i)
	{
		for (int j = 0; j < numSlices; ++j)
		{
			int stride = numSlices + 1;
			int BL = (i * stride) + j;
			int BR = BL + 1;
			int TR = BL + stride + 1;
			int TL = BL + stride;

			AABB2 quadUVs(uvs[BL], uvs[TR]);

			AddVertsForQuad3D(verts, positions[BL], positions[BR], positions[TR], positions[TL], color, quadUVs);
		}
	}
}

void AddVertsForSphere3D(
	std::vector<Vertex_PCUTBN>& verts,
	std::vector<unsigned int>& indices,
	const Vec3& center,
	float radius,
	const Rgba8& color,
	const AABB2& UVs,
	int numSlices,
	int numStacks)
{

	unsigned int startIndex = static_cast<unsigned int>(verts.size());

	for (int stack = 0; stack <= numStacks; ++stack)
	{
		float v = static_cast<float>(stack) / numStacks;
		float phi = v * (float)PI;

		for (int slice = 0; slice <= numSlices; ++slice)
		{
			float u = static_cast<float>(slice) / numSlices;
			float theta = u * 2.0f * (float)PI;

			float sinPhi = sinf(phi);
			float cosPhi = cosf(phi);
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);

			Vec3 position
			(
				radius * sinPhi * cosTheta,
				radius * sinPhi * sinTheta,
				radius * cosPhi
			);

			position += center;

			Vec3 normal = (position - center).GetNormalized();

			Vec3 tangent;
			if (stack == 0 || stack == numStacks) 
			{
				tangent = Vec3(cosTheta, sinTheta, 0.0f); 
			}
			else
			{
				tangent = Vec3(
					-sinPhi * sinTheta,
					sinPhi * cosTheta,
					0.0f
				);
			}
			tangent = tangent.GetNormalized();

			Vec3 bitangent = CrossProduct3D(normal, tangent).GetNormalized();

			Vec2 uv
			(
				UVs.m_mins.x + u * (UVs.m_maxs.x - UVs.m_mins.x),
				UVs.m_mins.y + v * (UVs.m_maxs.y - UVs.m_mins.y)
			);

			verts.push_back(Vertex_PCUTBN(position, color, uv, tangent, bitangent, normal));
		}
	}

	for (int stack = 0; stack < numStacks; ++stack)
	{
		for (int slice = 0; slice < numSlices; ++slice)
		{
			int first = (stack * (numSlices + 1)) + slice;
			int second = first + numSlices + 1;

			indices.push_back(startIndex + first);
			indices.push_back(startIndex + second);
			indices.push_back(startIndex + first + 1);

			indices.push_back(startIndex + first + 1);
			indices.push_back(startIndex + second);
			indices.push_back(startIndex + second + 1);
		}
	}
}




void AddVertsForSkySphere3D(std::vector<Vertex_PCU>& verts, const Rgba8& color, int numSlices, int numStacks)
{
	const float radius = 1.0f;
	const Vec3 center = Vec3::ZERO;

	std::vector<Vec3> positions;
	std::vector<Vec2> uvs;

	float deltaTheta = 360.0f / static_cast<float>(numSlices);
	float deltaPhi = 180.0f / static_cast<float>(numStacks);

	for (int stack = 0; stack <= numStacks; ++stack)
	{
		float phi = static_cast<float>(stack) * deltaPhi;
		float v = static_cast<float>(stack) / static_cast<float>(numStacks); // [0,1]

		for (int slice = 0; slice <= numSlices; ++slice)
		{
			float theta = static_cast<float>(slice) * deltaTheta;
			float u = static_cast<float>(slice) / static_cast<float>(numSlices); // [0,1]

			float x = radius * SinDegrees(phi) * CosDegrees(theta);
			float y = radius * SinDegrees(phi) * SinDegrees(theta);
			float z = radius * CosDegrees(phi);

			Vec3 position = Vec3(x, y, z) + center;
			Vec2 uv(u, 1.0f - v); 

			positions.push_back(position);
			uvs.push_back(uv);
		}
	}

	for (int i = 0; i < numStacks; ++i)
	{
		for (int j = 0; j < numSlices; ++j)
		{
			int stride = numSlices + 1;
			int BL = (i * stride) + j;
			int BR = BL + 1;
			int TR = BL + stride + 1;
			int TL = BL + stride;

			verts.push_back(Vertex_PCU(positions[BL], color, uvs[BL]));
			verts.push_back(Vertex_PCU(positions[TL], color, uvs[TL]));
			verts.push_back(Vertex_PCU(positions[TR], color, uvs[TR]));

			verts.push_back(Vertex_PCU(positions[BL], color, uvs[BL]));
			verts.push_back(Vertex_PCU(positions[TR], color, uvs[TR]));
			verts.push_back(Vertex_PCU(positions[BR], color, uvs[BR]));
		}
	}
}



void AddVertsForUVSphereZWireframe3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, float numStacks, float lineThickness, Rgba8 const& tint)
{
	float deltaTheta = 180.f / numStacks;  
	float deltaPhi = 360.f / numStacks;

	for (int i = 1; i < numStacks; ++i) 
	{
		float theta = deltaTheta * i;
		float ringRadius = radius * SinDegrees(theta);
		float z = center.z + radius * CosDegrees(theta);

		Vec3 firstPoint = Vec3(center.x + ringRadius, center.y, z);
		Vec3 prevPoint = firstPoint;

		for (int j = 1; j <= numStacks; ++j)
		{
			float phi = deltaPhi * j;
			Vec3 nextPoint = Vec3(center.x + ringRadius * CosDegrees(phi), center.y + ringRadius * SinDegrees(phi), z);
			AddVertsForLineSegment3D(verts, prevPoint, nextPoint, lineThickness, tint);
			prevPoint = nextPoint;
		}

		AddVertsForLineSegment3D(verts, prevPoint, firstPoint, lineThickness, tint);
	}

	for (int j = 0; j < numStacks; ++j)
	{
		float phi = deltaPhi * j;
		float cosPhi = CosDegrees(phi);
		float sinPhi = SinDegrees(phi);

		Vec3 top = Vec3(center.x, center.y, center.z + radius);
		Vec3 bottom = Vec3(center.x, center.y, center.z - radius);
		Vec3 prevPoint = top;

		for (int i = 1; i < numStacks; ++i)
		{
			float theta = deltaTheta * i;
			Vec3 nextPoint = Vec3(center.x + radius * SinDegrees(theta) * cosPhi,
				center.y + radius * SinDegrees(theta) * sinPhi,
				center.z + radius * CosDegrees(theta));
			AddVertsForLineSegment3D(verts, prevPoint, nextPoint, lineThickness, tint);
			prevPoint = nextPoint;
		}

		AddVertsForLineSegment3D(verts, prevPoint, bottom, lineThickness, tint); 
	}
}




void AddVertsForArrow3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, float shaftPercentage, const Rgba8& color)
{
	Vec3 direction = (end - start).GetNormalized();
	float arrowLength = (end - start).GetLength();

	float shaftLength = arrowLength * shaftPercentage;
	Vec3 shaftEnd = start + direction * shaftLength;

	Vec3 headEnd = end;

	float shaftRadius = radius;

	float headRadius = radius * 1.5f;

	std::vector<Vertex_PCU> shaftVerts;
	AddVertsForCylinder3D(shaftVerts, start, shaftEnd, shaftRadius, color, AABB2(Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f)), 16);

	std::vector<Vertex_PCU> headVerts;
	AddVertsForCone3D(headVerts, shaftEnd, headEnd, headRadius, color, AABB2(Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f)), 16);

	verts.insert(verts.end(), shaftVerts.begin(), shaftVerts.end());
	verts.insert(verts.end(), headVerts.begin(), headVerts.end());
}
