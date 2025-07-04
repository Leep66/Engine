#include "SpriteAnimDefinition.hpp"

SpriteAnimDefinition::SpriteAnimDefinition(SpriteSheet const& sheet, int startSpriteIndex, int endSpriteIndex, float framesPerSecond, SpriteAnimPlaybackType playbackType)
	:m_spriteSheet(sheet)
	,m_startSpriteIndex(startSpriteIndex)
	,m_endSpriteIndex(endSpriteIndex)
	,m_framesPerSecond(framesPerSecond)
	,m_playbackType(playbackType)
{

}

SpriteDefinition const& SpriteAnimDefinition::GetSpriteDefAtTime(float seconds) const
{
	int totalFrames = m_endSpriteIndex - m_startSpriteIndex + 1;
	float frameDuration = m_framesPerSecond;
	int frameIndex = 0;

	switch (m_playbackType)
	{
	case SpriteAnimPlaybackType::ONCE:
		frameIndex = std::min((int)(seconds / frameDuration), totalFrames - 1);
		break;

	case SpriteAnimPlaybackType::LOOP:
		frameIndex = (int)(seconds / frameDuration) % totalFrames;
		break;

	case SpriteAnimPlaybackType::PINGPONG:
	{
		int frame = (int)(seconds / frameDuration) % (totalFrames * 2 - 2);
		frameIndex = frame < totalFrames ? frame : totalFrames * 2 - 2 - frame;
		break;
	}
	}
	return m_spriteSheet.GetSpriteDef(m_startSpriteIndex + frameIndex);
}


float SpriteAnimDefinition::GetAnimRequiredSeconds() const
{
	return m_framesPerSecond * (m_endSpriteIndex + 1 - m_startSpriteIndex);
}