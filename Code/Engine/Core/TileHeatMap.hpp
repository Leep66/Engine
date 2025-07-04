#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"

#include <vector>

class FloatRange;

class TileHeatMap
{
public:
	TileHeatMap(IntVec2 const& dimensions, float initialValue = 0.f);
	~TileHeatMap() = default;

	int GetNumTiles() const { return m_dimensions.x * m_dimensions.y; }

	void SetValueForAllTiles(float maxValue);
	void SetValueAtIndex(int tileIndex, float value);

	float GetValueAtIndex(int tileIndex) const;

	void AddHeatVertsForDebugDraw(std::vector<Vertex_PCU>& verts, AABB2 totalBounds, 
		FloatRange valueRange, Rgba8 lowColor, Rgba8 highColor, float specialValue, Rgba8 specialColor) const;

	void AddSolidVertsForDebugDraw(std::vector<Vertex_PCU>& verts, AABB2 totalBounds, Rgba8 lowColor, float specialValue, Rgba8 specialColor) const;
	float GetMaxValue(float specialValue) const;
public:
	float* m_values = nullptr;
	IntVec2 m_dimensions = IntVec2(0, 0);
	std::vector<float> m_costs;


};