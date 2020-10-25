#pragma once
#include "shared/types.h"

struct vec;
struct vec4;

struct vec2
{
    union 
    {
        struct 
        {
            float x;
            float y;
        };
        float v[2];
    };
    vec2() {}
    vec2(float x, float y) : x(x), y(y) {};
    explicit vec2(const vec &v);
    explicit vec2(const vec4 &v);
    float & operator[](int);
    float operator[](int) const;
    bool operator==(const vec2 &) const;
    bool operator!=(const vec2 &) const;
    bool iszero() const;
    float dot(const vec2 &) const;
    float squaredlen() const;
    float magnitude() const;
    vec2 & normalize();
    vec2 & safenormalize();
    float cross(const vec2 &) const;
    float squaredist(const vec2 &) const;
    float dist(const vec2 &) const;
    vec2 & mul(float);
    vec2 & mul(const vec2 &);
    vec2 & square();
    vec2 & div(float);
    vec2 & div(const vec2 &);
    vec2 & recip();
    vec2 & add(float);
    vec2 & add(const vec2 &);
    vec2 & sub(float);
    vec2 & sub(const vec2 &);
    vec2 & neg();
    vec2 & min(const vec2 &);
    vec2 & max(const vec2 &);
    vec2 & min(float);
    vec2 & max(float);
    vec2 & abs();
    vec2 & clamp(float, float);
    vec2 & reflect(const vec2 &);
    vec2 & lerp(const vec2 &, float);
    vec2 & lerp(const vec2 &, const vec2 &, float);
    vec2 & avg(const vec2 &);
    vec2 & rotate_around_z(float, float);
    vec2 & rotate_around_z(float);
    vec2 & rotate_around_z(const vec2 &);
};

static inline bool htcmp(const vec2 &x, const vec2 &y)
{
    return x == y;
}

static inline uint hthash(const vec2 &k)
{
    union { uint i; float f; } x, y;
    x.f = k.x; y.f = k.y;
    uint v = x.i^y.i;
    return v + (v>>12);
}


