#pragma once
#include "half.h"


struct vec2;

struct hvec2
{
    half x, y;

    hvec2() {}
    hvec2(float x, float y) : x(x), y(y) {}
    hvec2(const vec2 &v);

    bool operator==(const hvec2 &h) const { return x == h.x && y == h.y; }
    bool operator!=(const hvec2 &h) const { return x != h.x || y != h.y; }
};
