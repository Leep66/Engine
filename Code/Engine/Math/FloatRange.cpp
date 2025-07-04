#include "Engine/Math/FloatRange.hpp"

const FloatRange FloatRange::ZERO(0.0f, 0.0f);
const FloatRange FloatRange::ONE(1.0f, 1.0f);
const FloatRange FloatRange::ZERO_TO_ONE(0.0f, 1.0f);

FloatRange::FloatRange() 
	: m_min(0.0f), m_max(0.0f) 
{

}

FloatRange::FloatRange(float min, float max) 
	: m_min(min), m_max(max) 
{

}

FloatRange& FloatRange::operator=(const FloatRange& other) {
	m_min = other.m_min;
	m_max = other.m_max;

    return *this;
}

bool FloatRange::operator==(const FloatRange& other) const
{
	return (m_min == other.m_min) && (m_max == other.m_max);
}

bool FloatRange::operator!=(const FloatRange& other) const
{
	return (m_min != other.m_min) || (m_max != other.m_max);
}

bool FloatRange::IsInRange(float value) const
{
	return (value >= m_min) && (value <= m_max);
}

bool FloatRange::IsOverlappingWith(const FloatRange& other) const
{
	return !(m_max < other.m_min) || (m_min > other.m_max);
}

