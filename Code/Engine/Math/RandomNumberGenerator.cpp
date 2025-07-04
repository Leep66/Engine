#include "Engine/Math/RandomNumberGenerator.hpp"

#include <stdlib.h>

int RandomNumberGenerator::RollRandomIntLessThan(int maxNotInclusive)
{
	return rand() % maxNotInclusive;
}

int RandomNumberGenerator::RollRandomIntInRange(int minInclusive, int maxInclusive)
{
	int range = (maxInclusive - minInclusive) + 1;
	return minInclusive + RollRandomIntLessThan(range);
}

float RandomNumberGenerator::RollRandomFloatZeroToOne()
{
	return static_cast<float>(rand()) / static_cast<float>(32767);
}



float RandomNumberGenerator::RollRandomFloatInRange(float minInclusive, float maxInclusive)
{
	float zeroToOne = RollRandomFloatZeroToOne();
	return minInclusive + (zeroToOne * (maxInclusive - minInclusive));
}

bool RandomNumberGenerator::RollPercentChance(float percentage)
{
	return RollRandomFloatZeroToOne() <= percentage;
}

float RandomNumberGenerator::RollRandomFloatInRange(FloatRange range)
{
	return RollRandomFloatInRange(range.m_min, range.m_max);
}



