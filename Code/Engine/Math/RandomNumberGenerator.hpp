#pragma once
#include "Engine/Math/FloatRange.hpp"

class RandomNumberGenerator
{
public:
	int RollRandomIntLessThan(int maxNotInclusive);
	int RollRandomIntInRange(int minInclusive, int maxInclusive);
	float RollRandomFloatZeroToOne();
	float RollRandomFloatInRange(float minInclusive, float maxInclusive);
	bool RollPercentChance(float percentage);
	float RollRandomFloatInRange(FloatRange range);
private:
	// unsigned int m_seed = 0;
	// int m_position = 0;
};