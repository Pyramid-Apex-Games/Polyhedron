#include "squat.h"
#include "vec4.h"

squat::squat(const vec4 &q) { convert(q); }

void squat::convert(const vec4 &q)
{
    x = short(q.x*32767.5f-0.5f);
    y = short(q.y*32767.5f-0.5f);
    z = short(q.z*32767.5f-0.5f);
    w = short(q.w*32767.5f-0.5f);
}

void squat::lerp(const vec4 &a, const vec4 &b, float t)
{
    vec4 q;
    q.lerp(a, b, t);
    convert(q);
}