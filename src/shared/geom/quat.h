#pragma once
#include "vec4.h"
#include "matrix3.h"
#include "matrix4.h"
#include <math.h>

struct vec;
struct matrix3;
struct matrix4x3;
struct matrix4;

struct quat : vec4
{
    quat() {}
    quat(float x, float y, float z, float w);
    quat(const vec &axis, float angle);
    quat(const vec &u, const vec &v);
    explicit quat(const vec &v);
    explicit quat(const matrix3 &m) { convertmatrix(m); }
    explicit quat(const matrix4x3 &m) { convertmatrix(m); }
    explicit quat(const matrix4 &m) { convertmatrix(m); }

    void restorew();

    quat &add(const vec4 &o);
    quat &sub(const vec4 &o);
    quat &mul(float k);
    template<class B> quat &madd(const vec4 &a, const B &b) { return add(vec4(a).mul(b)); }
    template<class B> quat &msub(const vec4 &a, const B &b) { return sub(vec4(a).mul(b)); }

    quat &mul(const quat &p, const quat &o)
    {
        x = p.w*o.x + p.x*o.w + p.y*o.z - p.z*o.y;
        y = p.w*o.y - p.x*o.z + p.y*o.w + p.z*o.x;
        z = p.w*o.z + p.x*o.y - p.y*o.x + p.z*o.w;
        w = p.w*o.w - p.x*o.x - p.y*o.y - p.z*o.z;
        return *this;
    }
    quat &mul(const quat &o) { return mul(quat(*this), o); }

    quat &invert() { neg3(); return *this; }

    quat &normalize() { vec4::normalize(); return *this; }

    void calcangleaxis(float &angle, vec &axis) const;

    vec calcangles() const;

    vec rotate(const vec &v) const;

    vec invertedrotate(const vec &v) const;

    template<class M>
    void convertmatrix(const M &m)
    {
        float trace = m.a.x + m.b.y + m.c.z;
        if(trace>0)
        {
            float r = sqrtf(1 + trace), inv = 0.5f/r;
            w = 0.5f*r;
            x = (m.b.z - m.c.y)*inv;
            y = (m.c.x - m.a.z)*inv;
            z = (m.a.y - m.b.x)*inv;
        }
        else if(m.a.x > m.b.y && m.a.x > m.c.z)
        {
            float r = sqrtf(1 + m.a.x - m.b.y - m.c.z), inv = 0.5f/r;
            x = 0.5f*r;
            y = (m.a.y + m.b.x)*inv;
            z = (m.c.x + m.a.z)*inv;
            w = (m.b.z - m.c.y)*inv;
        }
        else if(m.b.y > m.c.z)
        {
            float r = sqrtf(1 + m.b.y - m.a.x - m.c.z), inv = 0.5f/r;
            x = (m.a.y + m.b.x)*inv;
            y = 0.5f*r;
            z = (m.b.z + m.c.y)*inv;
            w = (m.c.x - m.a.z)*inv;
        }
        else
        {
            float r = sqrtf(1 + m.c.z - m.a.x - m.b.y), inv = 0.5f/r;
            x = (m.c.x + m.a.z)*inv;
            y = (m.b.z + m.c.y)*inv;
            z = 0.5f*r;
            w = (m.a.y - m.b.x)*inv;
        }
    }

};