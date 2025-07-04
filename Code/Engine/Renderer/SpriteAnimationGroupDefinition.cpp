#include "SpriteAnimationGroupDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

SpriteAnimDefinition* SpriteAnimationGroupDefinition::GetSpriteAnimDef(const Vec3& facingDir) const
{
	float bestDot = -1.f;
	const AnimDirection* bestDir = nullptr;

	if ((int)m_animDirections.size() == 1)
	{
		return m_animDirections[0].m_spriteAnimDef;
	}

	for (AnimDirection const& dir : m_animDirections)
	{
		float dot = DotProduct3D(dir.m_vector.GetNormalized(), facingDir.GetNormalized());
		if (dot > bestDot)
		{
			bestDot = dot;
			bestDir = &dir;
		}
	}

	if (bestDir)
	{
		return bestDir->m_spriteAnimDef;
	}
	return nullptr;
}


bool SpriteAnimationGroupDefinition::LoadFromXmlElement(const XmlElement& element, const SpriteSheet& spriteSheet)
{
	std::string agName = ParseXmlAttribute(element, "name", "");
	float secondsPerFrame = ParseXmlAttribute(element, "secondsPerFrame", 1.f);
	std::string playbackName = ParseXmlAttribute(element, "playbackMode", "");
	bool scaleBySpeed = ParseXmlAttribute(element, "scaleBySpeed", false);

	SpriteAnimPlaybackType playbackMode = SpriteAnimPlaybackType::ONCE;

	if (playbackName == "Once")
	{
		playbackMode = SpriteAnimPlaybackType::ONCE;
	}
	else if (playbackName == "Loop")
	{
		playbackMode = SpriteAnimPlaybackType::LOOP;
	}
	else if (playbackName == "PingPong")
	{
		playbackMode = SpriteAnimPlaybackType::PINGPONG;
	}

	m_name = agName;
	m_secondsPerFrame = secondsPerFrame;
	m_playbackMode = playbackMode;
	m_scaleBySpeed = scaleBySpeed;


	XmlElement const* dirElem = element.FirstChildElement("Direction");

	while (dirElem)
	{
		Vec3 vector = ParseXmlAttribute(*dirElem, "vector", Vec3(1.f, 0.f, 0.f));

		AnimDirection direction;
		direction.m_vector = vector;

		XmlElement const* animElem = dirElem->FirstChildElement("Animation");

		if (animElem)
		{
			int startFrame = ParseXmlAttribute(*animElem, "startFrame", 1);
			int endFrame = ParseXmlAttribute(*animElem, "endFrame", 1);

			SpriteAnimDefinition* animDef = new SpriteAnimDefinition(spriteSheet, startFrame, endFrame, secondsPerFrame, playbackMode);

			direction.m_spriteAnimDef = animDef;
		}

		m_animDirections.push_back(direction);
		dirElem = dirElem->NextSiblingElement("Direction");
	}

	return true;
}