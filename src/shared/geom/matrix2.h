#pragma once
#include "vec2.h"

struct matrix3;
struct matrix4;

struct matrix2
{
    vec2 a, b;

    matrix2() {}
    matrix2(const vec2 &a, const vec2 &b) : a(a), b(b) {}
    explicit matrix2(const matrix4 &m);
    explicit matrix2(const matrix3 &m);
};
