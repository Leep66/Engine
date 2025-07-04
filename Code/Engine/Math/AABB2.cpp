#include "AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"

const AABB2 AABB2::ZERO_TO_ONE(Vec2(0, 0), Vec2(1, 1));
const AABB2 AABB2::ZERO(Vec2(0, 0), Vec2(0, 0));
AABB2::AABB2(AABB2 const& copyFrom)
	: m_mins(copyFrom.m_mins)
	, m_maxs(copyFrom.m_maxs)
{
}

AABB2::AABB2(float minX, float minY, float maxX, float maxY)
	: m_mins(Vec2(minX, minY))
	, m_maxs(Vec2(maxX, maxY))
{
}

AABB2::AABB2(Vec2 const& mins, Vec2 const& maxs)
	: m_mins(mins)
	, m_maxs(maxs)
{
}

bool AABB2::IsPointInside(Vec2 const& point) const
{
	bool insideX = (point.x >= m_mins.x) && (point.x <= m_maxs.x);
	bool insideY = (point.y >= m_mins.y) && (point.y <= m_maxs.y);

	return insideX && insideY;

}

Vec2 const AABB2::GetCenter() const
{
	return Vec2((m_mins.x + m_maxs.x) * 0.5f, (m_mins.y + m_maxs.y) * 0.5f);
}

Vec2 const AABB2::GetDimensions() const
{
	return Vec2((m_maxs.x - m_mins.x), (m_maxs.y - m_mins.y));
}

float AABB2::GetAspect() const
{
	float width = m_maxs.x - m_mins.x;
	float height = m_maxs.y - m_mins.y;

	return width / height;
}


Vec2 const AABB2::GetNearestPoint(Vec2 const& referencePosition) const
{
    float nearestX = referencePosition.x;
    float nearestY = referencePosition.y;

    if (nearestX < m_mins.x) {
        nearestX = m_mins.x;
    }
    else if (nearestX > m_maxs.x) {
        nearestX = m_maxs.x;
    }

    if (nearestY < m_mins.y) {
        nearestY = m_mins.y;
    }
    else if (nearestY > m_maxs.y) {
        nearestY = m_maxs.y;
    }

    return Vec2(nearestX, nearestY);
}

Vec2 const AABB2::GetPointAtUV(Vec2 const& uv) const
{
    float x = m_mins.x + uv.x * (m_maxs.x - m_mins.x);
    float y = m_mins.y + uv.y * (m_maxs.y - m_mins.y);

    return Vec2(x, y);
}

Vec2 const AABB2::GetUVForPoint(Vec2 const& point) const
{
    float u = (point.x - m_mins.x) / (m_maxs.x - m_mins.x);
    float v = (point.y - m_mins.y) / (m_maxs.y - m_mins.y);

    return Vec2(u, v);
}

AABB2 AABB2::GetBoxAtUVs(Vec2 uvMins, Vec2 uvMaxs) const
{
	Vec2 newMins = Vec2
    (
		m_mins.x + (m_maxs.x - m_mins.x) * uvMins.x,
        m_mins.y + (m_maxs.y - m_mins.y) * uvMins.y
	);

	Vec2 newMaxs = Vec2
    (
        m_mins.x + (m_maxs.x - m_mins.x) * uvMaxs.x,
        m_mins.y + (m_maxs.y - m_mins.y) * uvMaxs.y
	);

	return AABB2(newMins, newMaxs);
}


void AABB2::Translate(Vec2 const& translationToApply)
{
    m_mins += translationToApply;
    m_maxs += translationToApply;
}

void AABB2::SetCenter(Vec2 const& newCenter)
{
    Vec2 oldCenter = GetCenter();
    m_mins += (newCenter - oldCenter);
    m_maxs += (newCenter - oldCenter);

}

void AABB2::SetDimensions(Vec2 const& newDimensions)
{
    Vec2 center = GetCenter();

    float newHalfWidth = newDimensions.x * 0.5f;
    float newHalfHeight = newDimensions.y * 0.5f;

    m_mins = Vec2(center.x - newHalfWidth, center.y - newHalfHeight);
    m_maxs = Vec2(center.x + newHalfWidth, center.y + newHalfHeight);
}

void AABB2::StretchToIncludePoint(Vec2 const& point)
{
    if (point.x < m_mins.x) {
        m_mins.x = point.x;
    }
    if (point.y < m_mins.y) {
        m_mins.y = point.y;
    }

    if (point.x > m_maxs.x) {
        m_maxs.x = point.x;
    }
    if (point.y > m_maxs.y) {
        m_maxs.y = point.y;
    }
}

void AABB2::ClampWithin(AABB2 const& containingBox)
{
	m_mins.x = (m_mins.x > containingBox.m_mins.x) ? m_mins.x : containingBox.m_mins.x;
	m_mins.y = (m_mins.y > containingBox.m_mins.y) ? m_mins.y : containingBox.m_mins.y;
	m_maxs.x = (m_maxs.x < containingBox.m_mins.x) ? m_maxs.x : containingBox.m_maxs.x;
	m_maxs.y = (m_maxs.y < containingBox.m_maxs.y) ? m_maxs.y : containingBox.m_maxs.y;
}


void AABB2::AddPadding(float xToaddOnBothSides, float yToAddToTopAndBottom)
{
	float halfX = xToaddOnBothSides * 0.5f;
	float halfY = yToAddToTopAndBottom * 0.5f;

	m_mins.x -= halfX;
	m_mins.y -= halfY;
	m_maxs.x += halfX;
	m_maxs.y += halfY;
}


void AABB2::ReduceToAspect(float newAspect)
{
	float currentWidth = m_maxs.x - m_mins.x;
	float currentHeight = m_maxs.y - m_mins.y;
	float currentAspect = currentWidth / currentHeight;

	Vec2 center = (m_mins + m_maxs) * 0.5f;

	if (currentAspect > newAspect)
	{
		float newWidth = currentHeight * newAspect;
		float halfNewWidth = newWidth * 0.5f;
		m_mins.x = center.x - halfNewWidth;
		m_maxs.x = center.x + halfNewWidth;
	}
	else
	{
		float newHeight = currentWidth / newAspect;
		float halfNewHeight = newHeight * 0.5f;
		m_mins.y = center.y - halfNewHeight;
		m_maxs.y = center.y + halfNewHeight;
	}
}


void AABB2::EnlargeToAspect(float newAspect)
{
	float currentWidth = m_maxs.x - m_mins.x;
	float currentHeight = m_maxs.y - m_mins.y;
	float currentAspect = currentWidth / currentHeight;

	Vec2 center = (m_mins + m_maxs) * 0.5f;

	if (currentAspect > newAspect)
	{
		float newHeight = currentWidth / newAspect;
		float halfNewHeight = newHeight * 0.5f;
		m_mins.y = center.y - halfNewHeight;
		m_maxs.y = center.y + halfNewHeight;
	}
	else
	{
		float newWidth = currentHeight * newAspect;
		float halfNewWidth = newWidth * 0.5f;
		m_mins.x = center.x - halfNewWidth;
		m_maxs.x = center.x + halfNewWidth;
	}
}

AABB2 AABB2::ChopOffTop(float percentOfOriginalToChop, float extraHeightOfOriginalToChop)
{
	float height = m_maxs.y - m_mins.y;
	float chopHeight = GetClamped(height * percentOfOriginalToChop + extraHeightOfOriginalToChop, 0.0f, height);

	AABB2 chopped(m_mins.x, m_maxs.y - chopHeight, m_maxs.x, m_maxs.y);
	m_maxs.y -= chopHeight;
	return chopped;
}

AABB2 AABB2::ChopOffBottom(float percentOfOriginalToChop, float extraHeightOfOriginalToChop)
{
	float height = m_maxs.y - m_mins.y;
	float chopHeight = GetClamped(height * percentOfOriginalToChop + extraHeightOfOriginalToChop, 0.0f, height);

	AABB2 chopped(m_mins.x, m_mins.y, m_maxs.x, m_mins.y + chopHeight);
	m_mins.y += chopHeight;
	return chopped;
}

AABB2 AABB2::ChopOffLeft(float percentOfOriginalToChop, float extraWidthOfOriginalToChop)
{
	float width = m_maxs.x - m_mins.x;
	float chopWidth = GetClamped(width * percentOfOriginalToChop + extraWidthOfOriginalToChop, 0.0f, width);

	AABB2 chopped(m_mins.x, m_mins.y, m_mins.x + chopWidth, m_maxs.y);
	m_mins.x += chopWidth;
	return chopped;
}

AABB2 AABB2::ChopOffRight(float percentOfOriginalToChop, float extraWidthOfOriginalToChop)
{
	float width = m_maxs.x - m_mins.x;
	float chopWidth = GetClamped(width * percentOfOriginalToChop + extraWidthOfOriginalToChop, 0.0f, width);

	AABB2 chopped(m_maxs.x - chopWidth, m_mins.y, m_maxs.x, m_maxs.y);
	m_maxs.x -= chopWidth;
	return chopped;
}

bool AABB2::operator==(AABB2 const& rightHandSideCompare) const
{
    return m_mins == rightHandSideCompare.m_mins
        && m_maxs == rightHandSideCompare.m_maxs;
}
