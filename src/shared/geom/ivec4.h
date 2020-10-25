#pragma once
#include "shared/types.h"
class ivec;
class vec4;

struct ivec4
{
    union
    {
        struct { int x, y, z, w; };
        struct { int r, g, b, a; };
        int v[4];
    };

    ivec4() {}
    explicit ivec4(const ivec &p, int w = 0);
    ivec4(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}
    explicit ivec4(const vec4 &v);

    int &operator[](int i)       { return v[i]; }
    int  operator[](int i) const { return v[i]; }

    bool operator==(const ivec4 &o) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
    bool operator!=(const ivec4 &o) const { return x != o.x || y != o.y || z != o.z || w != o.w; }
};

static inline bool htcmp(const ivec4 &x, const ivec4 &y)
{
    return x == y;
}

static inline uint hthash(const ivec4 &k)
{
    return k.x^k.y^k.z^k.w;
}