#pragma once
#include "vec.h"

struct vec4;

struct plane : vec
{
    float offset;

    float dist(const vec &p) const { return dot(p)+offset; }
    float dist(const vec4 &p) const;
    bool operator==(const plane &p) const { return x==p.x && y==p.y && z==p.z && offset==p.offset; }
    bool operator!=(const plane &p) const { return x!=p.x || y!=p.y || z!=p.z || offset!=p.offset; }

    plane() {}
    plane(const vec &c, float off) : vec(c), offset(off) {}
    plane(const vec4 &p);
    plane(int d, float off)
    {
        x = y = z = 0.0f;
        v[d] = 1.0f;
        offset = -off;
    }
    plane(float a, float b, float c, float d) : vec(a, b, c), offset(d) {}

    void toplane(const vec &n, const vec &p)
    {
        x = n.x; y = n.y; z = n.z;
        offset = -dot(p);
    }

    bool toplane(const vec &a, const vec &b, const vec &c)
    {
        cross(vec(b).sub(a), vec(c).sub(a));
        float mag = magnitude();
        if(!mag) return false;
        div(mag);
        offset = -dot(a);
        return true;
    }

    bool rayintersect(const vec &o, const vec &ray, float &dist)
    {
        float cosalpha = dot(ray);
        if(cosalpha==0) return false;
        float deltac = offset+dot(o);
        dist -= deltac/cosalpha;
        return true;
    }

    plane &reflectz(float rz)
    {
        offset += 2*rz*z;
        z = -z;
        return *this;
    }

    plane &invert()
    {
        neg();
        offset = -offset;
        return *this;
    }

    plane &scale(float k)
    {
        mul(k);
        return *this;
    }

    plane &translate(const vec &p)
    {
        offset += dot(p);
        return *this;
    }

    plane &normalize()
    {
        float mag = magnitude();
        div(mag);
        offset /= mag;
        return *this;
    }

    float zintersect(const vec &p) const { return -(x*p.x+y*p.y+offset)/z; }
    float zdelta(const vec &p) const { return -(x*p.x+y*p.y)/z; }
    float zdist(const vec &p) const { return p.z-zintersect(p); }
};
