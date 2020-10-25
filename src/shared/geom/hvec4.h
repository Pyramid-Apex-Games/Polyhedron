#pragma once
#include "half.h"

struct vec;
struct vec4;

struct hvec4
{
    half x, y, z, w;

    hvec4() {}
    hvec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    hvec4(const vec &v, float w);
    hvec4(const vec4 &v);

    bool operator==(const hvec4 &h) const { return x == h.x && y == h.y && z == h.z && w == h.w; }
    bool operator!=(const hvec4 &h) const { return x != h.x || y != h.y || z != h.z || w != h.w; }
};
