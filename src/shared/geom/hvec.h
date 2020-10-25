#pragma once
#include "half.h"

class vec;

struct hvec
{
    half x, y, z;

    hvec() {}
    hvec(float x, float y, float z) : x(x), y(y), z(z) {}
    hvec(const vec &v);

    bool operator==(const hvec &h) const { return x == h.x && y == h.y && z == h.z; }
    bool operator!=(const hvec &h) const { return x != h.x || y != h.y || z != h.z; }
};
