#include "Engine/Core/TileHeatMap.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/FloatRange.hpp"


TileHeatMap::TileHeatMap(IntVec2 const& dimensions, float initialValue)
	:m_dimensions(dimensions)
{
	int numTiles = dimensions.x * dimensions.y;
	m_values = new float[numTiles];

	for (int i = 0; i < numTiles; ++i)
	{
		m_values[i] = initialValue;
	}
}

void TileHeatMap::SetValueForAllTiles(float maxValue)
{
	for (int i = 0; i < GetNumTiles(); ++i)
	{
		m_values[i] = maxValue;
	}
}

void TileHeatMap::SetValueAtIndex(int tileIndex, float value)
{
	GUARANTEE_OR_DIE(tileIndex >= 0 && tileIndex < GetNumTiles(), "Bad tile Index");
	m_values[tileIndex] = value;
}

float TileHeatMap::GetValueAtIndex(int tileIndex) const
{
	return m_values[tileIndex];
}

void TileHeatMap::AddHeatVertsForDebugDraw(std::vector<Vertex_PCU>& verts, AABB2 totalBounds, 
	FloatRange valueRange, Rgba8 lowColor, Rgba8 highColor, float specialValue, Rgba8 specialColor) const
{
	float tileWidth = (totalBounds.m_maxs.x - totalBounds.m_mins.x) / (float)m_dimensions.x;
	float tileHeight = (totalBounds.m_maxs.y - totalBounds.m_mins.y) / (float)m_dimensions.y;

	int numTiles = m_dimensions.x * m_dimensions.y;
	int numTris = numTiles * 2;
	int numVerts = numTris * 3;
	verts.reserve(numVerts);

	for (int tileY = 0; tileY < m_dimensions.y; ++tileY)
	{
		for (int tileX = 0; tileX < m_dimensions.x; ++tileX)
		{
			Vec2 tileMins = totalBounds.m_mins + Vec2(static_cast<float>(tileX * tileWidth), static_cast<float>(tileY * tileHeight));
			Vec2 tileMaxs = tileMins + Vec2(tileWidth, tileHeight);
			AABB2 tileBounds(tileMins, tileMaxs);

			int tileIndex = tileX + (tileY * m_dimensions.x);
			float value = m_values[tileIndex];
			float fractionWithinRange = GetFractionWithinRange(value, valueRange.m_min, valueRange.m_max);
			fractionWithinRange = GetClampedZeroToOne(fractionWithinRange);
			Rgba8 color = Interpolate(lowColor, highColor, fractionWithinRange);

			if (value >= specialValue)
			{
				color = specialColor;
			}

			
			AddVertsForAABB2D(verts, tileBounds, color);
		}
	}
}

void TileHeatMap::AddSolidVertsForDebugDraw(std::vector<Vertex_PCU>& verts, AABB2 totalBounds, Rgba8 lowColor, float specialValue, Rgba8 specialColor) const
{
	float tileWidth = (totalBounds.m_maxs.x - totalBounds.m_mins.x) / (float)m_dimensions.x;
	float tileHeight = (totalBounds.m_maxs.y - totalBounds.m_mins.y) / (float)m_dimensions.y;

	int numTiles = m_dimensions.x * m_dimensions.y;
	int numTris = numTiles * 2;
	int numVerts = numTris * 3;
	verts.reserve(numVerts);

	for (int tileY = 0; tileY < m_dimensions.y; ++tileY)
	{
		for (int tileX = 0; tileX < m_dimensions.x; ++tileX)
		{
			Vec2 tileMins = totalBounds.m_mins + Vec2(static_cast<float>(tileX * tileWidth), static_cast<float>(tileY * tileHeight));
			Vec2 tileMaxs = tileMins + Vec2(tileWidth, tileHeight);
			AABB2 tileBounds(tileMins, tileMaxs);

			int tileIndex = tileX + (tileY * m_dimensions.x);
			
			Rgba8 color = lowColor;
			if (m_values[tileIndex] >= specialValue)
			{
				color = specialColor;
			}

			AddVertsForAABB2D(verts, tileBounds, color);
		}
	}
}


float TileHeatMap::GetMaxValue(float specialValue) const
{
	float maxValue = 0.f;

	for (int i = 0; i < m_dimensions.x * m_dimensions.y; ++i)
	{
		if (m_values[i] > maxValue && m_values[i] != specialValue)
		{
			maxValue = m_values[i];
		}
	}

	return maxValue;
}

