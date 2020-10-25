#include "matrix3.h"
#include "matrix4x3.h"
#include "matrix4.h"
#include "quat.h"
#include "vec2.h"
#include <algorithm>
#include <math.h>

matrix3::matrix3(const quat &q)
{
    float x = q.x, y = q.y, z = q.z, w = q.w,
            tx = 2*x, ty = 2*y, tz = 2*z,
            txx = tx*x, tyy = ty*y, tzz = tz*z,
            txy = tx*y, txz = tx*z, tyz = ty*z,
            twx = w*tx, twy = w*ty, twz = w*tz;
    a = vec(1 - (tyy + tzz), txy + twz, txz - twy);
    b = vec(txy - twz, 1 - (txx + tzz), tyz + twx);
    c = vec(txz + twy, tyz - twx, 1 - (txx + tyy));
}


matrix3::matrix3(const matrix4x3 &m) : a(m.a), b(m.b), c(m.c) {}

matrix3::matrix3(const matrix4 &m)
        : a(m.a), b(m.b), c(m.c)
{}

void matrix3::transpose()
{
    std::swap(a.y, b.x); std::swap(a.z, c.x);
    std::swap(b.z, c.y);
}

void matrix3::rotate(float angle, const vec &axis)
{
    rotate(cosf(angle), sinf(angle), axis);
}

void matrix3::setyaw(float angle)
{
    setyaw(cosf(angle), sinf(angle));
}

bool matrix3::calcangleaxis(float tr, float &angle, vec &axis, float threshold) const
{
    if(tr <= -1)
    {
        if(a.x >= b.y && a.x >= c.z)
        {
            float r = 1 + a.x - b.y - c.z;
            if(r <= threshold) return false;
            r = sqrtf(r);
            axis.x = 0.5f*r;
            axis.y = b.x/r;
            axis.z = c.x/r;
        }
        else if(b.y >= c.z)
        {
            float r = 1 + b.y - a.x - c.z;
            if(r <= threshold) return false;
            r = sqrtf(r);
            axis.y = 0.5f*r;
            axis.x = b.x/r;
            axis.z = c.y/r;
        }
        else
        {
            float r = 1 + b.y - a.x - c.z;
            if(r <= threshold) return false;
            r = sqrtf(r);
            axis.z = 0.5f*r;
            axis.x = c.x/r;
            axis.y = c.y/r;
        }
        angle = M_PI;
    }
    else if(tr >= 3)
    {
        axis = vec(0, 0, 1);
        angle = 0;
    }
    else
    {
        axis = vec(b.z - c.y, c.x - a.z, a.y - b.x);
        float r = axis.squaredlen();
        if(r <= threshold) return false;
        axis.mul(1/sqrtf(r));
        angle = acosf(0.5f*(tr - 1));
    }
    return true;
}

void matrix3::rotate_around_x(float angle) { rotate_around_x(cosf(angle), sinf(angle)); }
void matrix3::rotate_around_x(const vec2 &sc) { rotate_around_x(sc.x, sc.y); }

void matrix3::rotate_around_y(float angle) { rotate_around_y(cosf(angle), sinf(angle)); }
void matrix3::rotate_around_y(const vec2 &sc) { rotate_around_y(sc.x, sc.y); }

void matrix3::rotate_around_z(float angle) { rotate_around_z(cosf(angle), sinf(angle)); }
void matrix3::rotate_around_z(const vec2 &sc) { rotate_around_z(sc.x, sc.y); }

vec matrix3::transform(const vec2 &o) { return vec(a).mul(o.x).madd(b, o.y); }
vec matrix3::transposedtransform(const vec2 &o) const { return vec(a.dot2(o), b.dot2(o), c.dot2(o)); }
