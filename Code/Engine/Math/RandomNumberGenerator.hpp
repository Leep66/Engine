#pragma once
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include <random>

class RandomNumberGenerator 
{
private:
    std::mt19937 m_rng;

public:
    RandomNumberGenerator() 
    {
        std::random_device rd;
        m_rng.seed(rd());
    }

    explicit RandomNumberGenerator(unsigned int seed) 
    {
        m_rng.seed(seed);
    }

    int RollRandomIntLessThan(int maxNotInclusive) 
    {
        std::uniform_int_distribution<int> dist(0, maxNotInclusive - 1);
        return dist(m_rng);
    }

    int RollRandomIntInRange(int minInclusive, int maxInclusive) 
    {
        std::uniform_int_distribution<int> dist(minInclusive, maxInclusive);
        return dist(m_rng);
    }

    float RollRandomFloatZeroToOne() 
    {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(m_rng);
    }

    float RollRandomFloatInRange(float minInclusive, float maxInclusive) 
    {
        std::uniform_real_distribution<float> dist(minInclusive, maxInclusive);
        return dist(m_rng);
    }

    bool RollPercentChance(float percentage) 
    {
        return RollRandomFloatZeroToOne() <= percentage;
    }

    float RollRandomFloatInRange(const FloatRange& range) 
    {
        return RollRandomFloatInRange(range.m_min, range.m_max);
    }

	Vec2 RollRandomVec2InRange(const Vec2& minInclusive, const Vec2& maxInclusive)
	{
		float x = RollRandomFloatInRange(minInclusive.x, maxInclusive.x);
		float y = RollRandomFloatInRange(minInclusive.y, maxInclusive.y);
		return Vec2(x, y);
	}

	Vec3 RollRandomVec3InRange(const Vec3& minInclusive, const Vec3& maxInclusive)
	{
		float x = RollRandomFloatInRange(minInclusive.x, maxInclusive.x);
		float y = RollRandomFloatInRange(minInclusive.y, maxInclusive.y);
		float z = RollRandomFloatInRange(minInclusive.z, maxInclusive.z);
		return Vec3(x, y, z);
	}

};