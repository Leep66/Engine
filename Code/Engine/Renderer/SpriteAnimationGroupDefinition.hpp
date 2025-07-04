#pragma once
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <string>
#include <vector>


struct AnimDirection
{
	Vec3 m_vector = Vec3(1.f, 0.f, 0.f);

	SpriteAnimDefinition* m_spriteAnimDef = nullptr;
};

struct SpriteAnimationGroupDefinition
{
	std::string m_name = "";
	bool m_scaleBySpeed = false;
	float m_secondsPerFrame = 0.f;
	SpriteAnimPlaybackType m_playbackMode = SpriteAnimPlaybackType::ONCE;

	std::vector<AnimDirection> m_animDirections;

	SpriteAnimDefinition* GetSpriteAnimDef(const Vec3& vector) const;
	bool LoadFromXmlElement(const XmlElement& element, const SpriteSheet& spriteSheet);
};