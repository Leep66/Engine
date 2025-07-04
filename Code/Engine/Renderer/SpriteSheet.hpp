#pragma once
#include "Engine/Math/IntVec2.hpp"
#include <vector>


struct AABB2;
struct Vec2;
class SpriteDefinition;
class Texture;

class SpriteSheet
{
public:
	explicit SpriteSheet(Texture& texture, IntVec2 const& simpleGridLayout);

	Texture& GetTexture() const;
	int GetNumSprites() const;
	SpriteDefinition const& GetSpriteDef(int spriteIndex) const;
	SpriteDefinition const& GetSpriteDef(IntVec2 spriteCoords) const;
	void GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const;
	AABB2 GetSpriteUVs(int spriteIndex) const;

protected:
	Texture& m_texture;
	IntVec2 m_simpleGridLayout;
	std::vector<SpriteDefinition> m_spriteDefs;
};