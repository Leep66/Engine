#include "Disc2.hpp"

Disc2::Disc2(Vec2 const& center, float radius)
	:m_center(center), m_radius(radius)
{

}


void Disc2::Translate(Vec2 const& translationToApply)
{
    m_center += translationToApply;
}

void  Disc2::SetCenter(Vec2 const& newCenter)
{
    m_center = newCenter;
}
