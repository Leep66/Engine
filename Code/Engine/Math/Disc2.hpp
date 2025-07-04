#pragma once

#include "Engine/Math/Vec2.hpp"

struct Disc2
{
public:
    Vec2 m_center;
    float m_radius = 0.f;

public:
    Disc2() = default;
    Disc2(Vec2 const& center, float radius);

    void Translate(Vec2 const& translationToApply);
    void SetCenter(Vec2 const& newCenter);
};