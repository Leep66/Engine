#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

SpriteSheet::SpriteSheet(Texture& texture, IntVec2 const& simpleGridLayout)
	: m_texture(texture)
	, m_simpleGridLayout(simpleGridLayout)
{
	Vec2 spriteSize(1.0f / simpleGridLayout.x, 1.0f / simpleGridLayout.y);
	Vec2 nudgeAmount(1.f / 262144.f, 1.f / 65536.f);
	
	for (int y = 0; y < simpleGridLayout.y; ++y)
	{
		for (int x = 0; x < simpleGridLayout.x; ++x)
		{
			int spriteIndex = x + y * simpleGridLayout.x;

			Vec2 uvAtMins((float)(x)*spriteSize.x, 1.f - ((float)(y + 1) * spriteSize.y));
			Vec2 uvAtMaxs(uvAtMins.x + spriteSize.x, 1.f - (float)(y)*spriteSize.y);

			uvAtMins += nudgeAmount;
			uvAtMaxs -= nudgeAmount;

			m_spriteDefs.push_back(SpriteDefinition(*this, spriteIndex, uvAtMins, uvAtMaxs));
		}
	}
}

Texture& SpriteSheet::GetTexture() const
{
	return m_texture;
}

int SpriteSheet::GetNumSprites() const
{
	return static_cast<int>(m_spriteDefs.size());
}

SpriteDefinition const& SpriteSheet::GetSpriteDef(int spriteIndex) const
{
	return m_spriteDefs[spriteIndex];
}

SpriteDefinition const& SpriteSheet::GetSpriteDef(IntVec2 spriteCoords) const
{
	int index = spriteCoords.y * m_simpleGridLayout.x + spriteCoords.x;
	GUARANTEE_OR_DIE(index >= 0 && index < static_cast<int> (m_spriteDefs.size()), "Invalid sprite coordinates!");

	return m_spriteDefs[index];

}

void SpriteSheet::GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const
{
	m_spriteDefs[spriteIndex].GetUVs(out_uvAtMins, out_uvAtMaxs);
}

AABB2 SpriteSheet::GetSpriteUVs(int spriteIndex) const
{
	Vec2 uvMins, uvMaxs;
	GetSpriteUVs(uvMins, uvMaxs, spriteIndex);
	return AABB2(uvMins, uvMaxs);
}



