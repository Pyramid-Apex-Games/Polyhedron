#pragma once
#include "shared/types.h"

struct vec2;
struct ivec;

struct ivec2
{
    union
    {
        struct { int x, y; };
        int v[2];
    };

    ivec2() {}
    ivec2(int x, int y) : x(x), y(y) {}
    explicit ivec2(const vec2 &v);
    explicit ivec2(const ivec &v);

    int &operator[](int i)       { return v[i]; }
    int  operator[](int i) const { return v[i]; }

    bool operator==(const ivec2 &o) const { return x == o.x && y == o.y; }
    bool operator!=(const ivec2 &o) const { return x != o.x || y != o.y; }

    bool iszero() const { return x==0 && y==0; }
    ivec2 &shl(int n) { x<<= n; y<<= n; return *this; }
    ivec2 &shr(int n) { x>>= n; y>>= n; return *this; }
    ivec2 &mul(int n) { x *= n; y *= n; return *this; }
    ivec2 &div(int n) { x /= n; y /= n; return *this; }
    ivec2 &add(int n) { x += n; y += n; return *this; }
    ivec2 &sub(int n) { x -= n; y -= n; return *this; }
    ivec2 &mul(const ivec2 &v) { x *= v.x; y *= v.y; return *this; }
    ivec2 &div(const ivec2 &v) { x /= v.x; y /= v.y; return *this; }
    ivec2 &add(const ivec2 &v) { x += v.x; y += v.y; return *this; }
    ivec2 &sub(const ivec2 &v) { x -= v.x; y -= v.y; return *this; }
    ivec2 &mask(int n) { x &= n; y &= n; return *this; }
    ivec2 &neg() { x = -x; y = -y; return *this; }
    ivec2 &min(const ivec2 &o);
    ivec2 &max(const ivec2 &o);
    ivec2 &min(int n);
    ivec2 &max(int n);
    ivec2 &abs();
    int dot(const ivec2 &o) const { return x*o.x + y*o.y; }
    int cross(const ivec2 &o) const { return x*o.y - y*o.x; }
};

static inline bool htcmp(const ivec2 &x, const ivec2 &y)
{
    return x == y;
}

static inline uint hthash(const ivec2 &k)
{
    return k.x^k.y;
}
