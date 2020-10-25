#pragma once

struct vec4;

struct squat
{
    short x, y, z, w;

    squat() {}
    squat(const vec4 &q);
    void convert(const vec4 &q);
    void lerp(const vec4 &a, const vec4 &b, float t);
};