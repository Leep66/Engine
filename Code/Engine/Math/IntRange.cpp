#include "Engine/Math/IntRange.hpp"

const IntRange IntRange::ZERO(0, 0);
const IntRange IntRange::ONE(1, 1);
const IntRange IntRange::ZERO_TO_ONE(0, 1);

IntRange::IntRange()
	:m_min(0), m_max(0)
{

}

IntRange::IntRange(float min, float max)
	: m_min(static_cast<int>(min)), m_max(static_cast<int>(max))
{

}

IntRange& IntRange::operator=(const IntRange& other)
{
	m_min = other.m_min;
	m_max = other.m_max;

	return *this;
}

bool IntRange::operator==(const IntRange& other) const
{	
	return (m_min == other.m_min) && (m_max == other.m_max);
}

bool IntRange::operator!=(const IntRange& other) const
{
	return (m_min != other.m_min) || (m_max != other.m_max);
}

bool IntRange::IsInRange(float value) const
{
	return (value >= m_min) && (value <= m_max);
}

bool IntRange::IsOverlappingWith(const IntRange& other) const
{
	return !(m_max < other.m_min) || (m_min > other.m_max);
}
