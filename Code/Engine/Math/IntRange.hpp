#pragma once

class IntRange {
public:
    int m_min;
    int m_max;

    static const IntRange ZERO;
    static const IntRange ONE;
    static const IntRange ZERO_TO_ONE;

    IntRange();
    explicit IntRange(float min, float max);
    IntRange& operator=(const IntRange& other);
    bool operator==(const IntRange& other) const;
    bool operator!=(const IntRange& other) const;

    bool IsInRange(float value) const;
    bool IsOverlappingWith(const IntRange& other) const;
};
