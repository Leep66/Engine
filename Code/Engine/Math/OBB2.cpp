#include "OBB2.hpp"
#include "Engine/Math/MathUtils.hpp"

OBB2::OBB2(const Vec2& center, const Vec2& iBasisNormal, const Vec2& halfDimensions)
    :m_center(center), m_iBasisNormal(iBasisNormal), m_halfDimensions(halfDimensions)
{
}

void OBB2::GetCornerPoints(Vec2* out_fourCornerWorldPositions) const
{
    Vec2 jBasisNormal = m_iBasisNormal.GetRotated90Degrees();
    Vec2 iScaled = m_iBasisNormal * m_halfDimensions.x;
    Vec2 jScaled = jBasisNormal * m_halfDimensions.y;

    out_fourCornerWorldPositions[0] = m_center - iScaled - jScaled;
    out_fourCornerWorldPositions[1] = m_center + iScaled - jScaled;
    out_fourCornerWorldPositions[2] = m_center + iScaled + jScaled;
    out_fourCornerWorldPositions[3] = m_center - iScaled + jScaled;
}

Vec2 const OBB2::GetLocalPosForWorldPos(Vec2 const& worldPos) const
{
    Vec2 displacement = worldPos - m_center;

    Vec2 jBasisNormal = m_iBasisNormal.GetRotated90Degrees();
    float localX = DotProduct2D(displacement, m_iBasisNormal);
    float localY = DotProduct2D(displacement, jBasisNormal);

    return Vec2(localX, localY);
}

Vec2 const OBB2::GetWorldPosForLocalPos(Vec2 const& localPos) const
{
    Vec2 jBasisNormal = m_iBasisNormal.GetRotated90Degrees();

    Vec2 worldOffset = (m_iBasisNormal * localPos.x) + (jBasisNormal * localPos.y);

    return m_center + worldOffset;
}

void OBB2::RotateAboutCenter(float rotationDeltaDegrees)
{
    float rotationDeltaRadians = ConvertDegreesToRadians(rotationDeltaDegrees);

    m_iBasisNormal.RotateDegrees(rotationDeltaRadians);
}






