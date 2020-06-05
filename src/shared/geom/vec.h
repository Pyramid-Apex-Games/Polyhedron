#pragma once
#include "shared/types.h"
#include "shared/loops.h"
#include <nlohmann/json_fwd.hpp>
#include <math.h>

struct vec2;
struct vec4;
struct ivec;
struct usvec;
struct svec;

struct vec
{
    union
    {
        struct { float x, y, z; };
        struct { float r, g, b; };
        float v[3];
    };

    vec() {}
    explicit vec(int a) : x(a), y(a), z(a) {}
    explicit vec(float a) : x(a), y(a), z(a) {}
    vec(float x, float y, float z) : x(x), y(y), z(z) {}
    explicit vec(int v[3]) : x(v[0]), y(v[1]), z(v[2]) {}
    explicit vec(const float *v) : x(v[0]), y(v[1]), z(v[2]) {}
    explicit vec(const vec2 &v, float z = 0);
    explicit vec(const vec4 &v);
    explicit vec(const ivec &v);
    explicit vec(const usvec &v);
    explicit vec(const svec &v);

    vec(float yaw, float pitch);

    float &operator[](int i)       { return v[i]; }
    float  operator[](int i) const { return v[i]; }

    vec &set(int i, float f) { v[i] = f; return *this; }

    bool operator==(const vec &o) const { return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const vec &o) const { return x != o.x || y != o.y || z != o.z; }

    bool iszero() const { return x==0 && y==0 && z==0; }
    float squaredlen() const { return x*x + y*y + z*z; }
    float dot2(const vec2 &o) const;
    float dot2(const vec &o) const { return x*o.x + y*o.y; }
    float dot(const vec &o) const { return x*o.x + y*o.y + z*o.z; }
    float squaredot(const vec &o) const { float k = dot(o); return k*k; }
    float absdot(const vec &o) const;
    float zdot(const vec &o) const { return z*o.z; }
    vec &mul(const vec &o)   { x *= o.x; y *= o.y; z *= o.z; return *this; }
    vec &mul(float f)        { x *= f; y *= f; z *= f; return *this; }
    vec &mul2(float f)       { x *= f; y *= f; return *this; }
    vec &square()            { mul(*this); return *this; }
    vec &div(const vec &o)   { x /= o.x; y /= o.y; z /= o.z; return *this; }
    vec &div(float f)        { x /= f; y /= f; z /= f; return *this; }
    vec &div2(float f)       { x /= f; y /= f; return *this; }
    vec &recip()             { x = 1/x; y = 1/y; z = 1/z; return *this; }
    vec &add(const vec &o)   { x += o.x; y += o.y; z += o.z; return *this; }
    vec &add(float f)        { x += f; y += f; z += f; return *this; }
    vec &add2(float f)       { x += f; y += f; return *this; }
    vec &addz(float f)       { z += f; return *this; }
    vec &sub(const vec &o)   { x -= o.x; y -= o.y; z -= o.z; return *this; }
    vec &sub(float f)        { x -= f; y -= f; z -= f; return *this; }
    vec &sub2(float f)       { x -= f; y -= f; return *this; }
    vec &subz(float f)       { z -= f; return *this; }
    vec &neg2()              { x = -x; y = -y; return *this; }
    vec &neg()               { x = -x; y = -y; z = -z; return *this; }
    vec &min(const vec &o);
    vec &max(const vec &o);
    vec &min(float f);
    vec &max(float f);
    vec &abs();
    vec &clamp(float l, float h);
    float magnitude2() const;
    float magnitude() const;
    vec &normalize()         { div(magnitude()); return *this; }
    vec &safenormalize()     { float m = magnitude(); if(m) div(m); return *this; }
    bool isnormalized() const { float m = squaredlen(); return (m>0.99f && m<1.01f); }
    float squaredist(const vec &e) const { return vec(*this).sub(e).squaredlen(); }
    float dist(const vec &e) const;
    float dist(const vec &e, vec &t) const { t = *this; t.sub(e); return t.magnitude(); }
    float dist2(const vec &o) const;
    template<class T>
    bool reject(const T &o, float r) { return x>o.x+r || x<o.x-r || y>o.y+r || y<o.y-r; }
    template<class A, class B>
    vec &cross(const A &a, const B &b) { x = a.y*b.z-a.z*b.y; y = a.z*b.x-a.x*b.z; z = a.x*b.y-a.y*b.x; return *this; }
    vec &cross(const vec &o, const vec &a, const vec &b) { return cross(vec(a).sub(o), vec(b).sub(o)); }
    float scalartriple(const vec &a, const vec &b) const { return x*(a.y*b.z-a.z*b.y) + y*(a.z*b.x-a.x*b.z) + z*(a.x*b.y-a.y*b.x); }
    float zscalartriple(const vec &a, const vec &b) const { return z*(a.x*b.y-a.y*b.x); }
    vec &reflectz(float rz) { z = 2*rz - z; return *this; }
    vec &reflect(const vec &n) { float k = 2*dot(n); x -= k*n.x; y -= k*n.y; z -= k*n.z; return *this; }
    vec &project(const vec &n) { float k = dot(n); x -= k*n.x; y -= k*n.y; z -= k*n.z; return *this; }
    vec &projectxydir(const vec &n) { if(n.z) z = -(x*n.x/n.z + y*n.y/n.z); return *this; }
    vec &projectxy(const vec &n);
    vec &projectxy(const vec &n, float threshold);
    vec &lerp(const vec &b, float t) { x += (b.x-x)*t; y += (b.y-y)*t; z += (b.z-z)*t; return *this; }
    vec &lerp(const vec &a, const vec &b, float t) { x = a.x + (b.x-a.x)*t; y = a.y + (b.y-a.y)*t; z = a.z + (b.z-a.z)*t; return *this; }
    vec &avg(const vec &b) { add(b); mul(0.5f); return *this; }
    template<class B> vec &madd(const vec &a, const B &b) { return add(vec(a).mul(b)); }
    template<class B> vec &msub(const vec &a, const B &b) { return sub(vec(a).mul(b)); }

    vec &rescale(float k)
    {
        float mag = magnitude();
        if(mag > 1e-6f) mul(k / mag);
        return *this;
    }

    vec &rotate_around_z(float c, float s) { float rx = x, ry = y; x = c*rx-s*ry; y = c*ry+s*rx; return *this; }
    vec &rotate_around_x(float c, float s) { float ry = y, rz = z; y = c*ry-s*rz; z = c*rz+s*ry; return *this; }
    vec &rotate_around_y(float c, float s) { float rx = x, rz = z; x = c*rx+s*rz; z = c*rz-s*rx; return *this; }

    vec &rotate_around_z(float angle);
    vec &rotate_around_x(float angle);
    vec &rotate_around_y(float angle);

    vec &rotate_around_z(const vec2 &sc);
    vec &rotate_around_x(const vec2 &sc);
    vec &rotate_around_y(const vec2 &sc);

    vec &rotate(float c, float s, const vec &d)
    {
        *this = vec(x*(d.x*d.x*(1-c)+c) + y*(d.x*d.y*(1-c)-d.z*s) + z*(d.x*d.z*(1-c)+d.y*s),
                    x*(d.y*d.x*(1-c)+d.z*s) + y*(d.y*d.y*(1-c)+c) + z*(d.y*d.z*(1-c)-d.x*s),
                    x*(d.x*d.z*(1-c)-d.y*s) + y*(d.y*d.z*(1-c)+d.x*s) + z*(d.z*d.z*(1-c)+c));
        return *this;
    }
    vec &rotate(float angle, const vec &d);
    vec &rotate(const vec2 &sc, const vec &d);

    void orthogonal(const vec &d);

    void orthonormalize(vec &s, vec &t) const
    {
        s.project(*this);
        t.project(*this).project(s);
    }

    template<class T> bool insidebb(const T &bbmin, const T &bbmax) const
    {
        return x >= bbmin.x && x <= bbmax.x && y >= bbmin.y && y <= bbmax.y && z >= bbmin.z && z <= bbmax.z;
    }

    template<class T, class U> bool insidebb(const T &bbmin, const T &bbmax, U margin) const
    {
        return x >= bbmin.x-margin && x <= bbmax.x+margin && y >= bbmin.y-margin && y <= bbmax.y+margin && z >= bbmin.z-margin && z <= bbmax.z+margin;
    }

    template<class T, class U> bool insidebb(const T &o, U size) const
    {
        return x >= o.x && x <= o.x + size && y >= o.y && y <= o.y + size && z >= o.z && z <= o.z + size;
    }

    template<class T, class U> bool insidebb(const T &o, U size, U margin) const
    {
        size += margin;
        return x >= o.x-margin && x <= o.x + size && y >= o.y-margin && y <= o.y + size && z >= o.z-margin && z <= o.z + size;
    }

    template<class T> float dist_to_bb(const T &min, const T &max) const
    {
        float sqrdist = 0;
        loopi(3)
        {
            if     (v[i] < min[i]) { float delta = v[i]-min[i]; sqrdist += delta*delta; }
            else if(v[i] > max[i]) { float delta = max[i]-v[i]; sqrdist += delta*delta; }
        }
        return sqrtf(sqrdist);
    }

    template<class T, class S> float dist_to_bb(const T &o, S size) const
    {
        return dist_to_bb(o, T(o).add(size));
    }

    template<class T> float project_bb(const T &min, const T &max) const
    {
        return x*(x < 0 ? max.x : min.x) + y*(y < 0 ? max.y : min.y) + z*(z < 0 ? max.z : min.z);
    }

    static vec hexcolor(int color)
    {
        return vec(((color>>16)&0xFF)*(1.0f/255.0f), ((color>>8)&0xFF)*(1.0f/255.0f), (color&0xFF)*(1.0f/255.0f));
    }

    int tohexcolor() const;
};

static inline bool htcmp(const vec &x, const vec &y)
{
    return x == y;
}

static inline uint hthash(const vec &k)
{
    union { uint i; float f; } x, y, z;
    x.f = k.x; y.f = k.y; z.f = k.z;
    uint v = x.i^y.i^z.i;
    return v + (v>>12);
}

void to_json(nlohmann::json& document, const vec& v);
void from_json(const nlohmann::json& document, vec& v);