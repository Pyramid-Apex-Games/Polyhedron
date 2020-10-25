#pragma once
#include <nlohmann/json_fwd.hpp>
struct vec2;
struct vec;

struct vec4
{
    union
    {
        struct { float x, y, z, w; };
        struct { float r, g, b, a; };
        float v[4];
    };

    vec4() {}
    explicit vec4(const vec &p, float w = 0);
    explicit vec4(const vec2 &p, float z = 0, float w = 0);
    vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    explicit vec4(const float *v) : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}

    float &operator[](int i)       { return v[i]; }
    float  operator[](int i) const { return v[i]; }

    bool operator==(const vec4 &o) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
    bool operator!=(const vec4 &o) const { return x != o.x || y != o.y || z != o.z || w != o.w; }

    float dot3(const vec4 &o) const { return x*o.x + y*o.y + z*o.z; }
    float dot3(const vec &o) const;
    float dot(const vec4 &o) const { return dot3(o) + w*o.w; }
    float dot(const vec &o) const;
    float squaredlen() const { return dot(*this); }
    float magnitude() const;
    float magnitude3() const;
    vec4 &normalize() { mul(1/magnitude()); return *this; }
    vec4 &safenormalize() { float m = magnitude(); if(m) mul(1/m); return *this; }

    vec4 &lerp(const vec4 &b, float t)
    {
        x += (b.x-x)*t;
        y += (b.y-y)*t;
        z += (b.z-z)*t;
        w += (b.w-w)*t;
        return *this;
    }
    vec4 &lerp(const vec4 &a, const vec4 &b, float t)
    {
        x = a.x+(b.x-a.x)*t;
        y = a.y+(b.y-a.y)*t;
        z = a.z+(b.z-a.z)*t;
        w = a.w+(b.w-a.w)*t;
        return *this;
    }
    vec4 &avg(const vec4 &b) { add(b); mul(0.5f); return *this; }
    template<class B> vec4 &madd(const vec4 &a, const B &b) { return add(vec4(a).mul(b)); }
    template<class B> vec4 &msub(const vec4 &a, const B &b) { return sub(vec4(a).mul(b)); }

    vec4 &mul3(float f)      { x *= f; y *= f; z *= f; return *this; }
    vec4 &mul(float f)       { mul3(f); w *= f; return *this; }
    vec4 &mul(const vec4 &o) { x *= o.x; y *= o.y; z *= o.z; w *= o.w; return *this; }
    vec4 &mul(const vec &o);
    vec4 &square()           { mul(*this); return *this; }
    vec4 &div3(float f)      { x /= f; y /= f; z /= f; return *this; }
    vec4 &div(float f)       { div3(f); w /= f; return *this; }
    vec4 &div(const vec4 &o) { x /= o.x; y /= o.y; z /= o.z; w /= o.w; return *this; }
    vec4 &div(const vec &o);
    vec4 &recip()            { x = 1/x; y = 1/y; z = 1/z; w = 1/w; return *this; }
    vec4 &add(const vec4 &o) { x += o.x; y += o.y; z += o.z; w += o.w; return *this; }
    vec4 &add(const vec &o);
    vec4 &add3(float f)      { x += f; y += f; z += f; return *this; }
    vec4 &add(float f)       { add3(f); w += f; return *this; }
    vec4 &addw(float f)      { w += f; return *this; }
    vec4 &sub(const vec4 &o) { x -= o.x; y -= o.y; z -= o.z; w -= o.w; return *this; }
    vec4 &sub(const vec &o);
    vec4 &sub3(float f)      { x -= f; y -= f; z -= f; return *this; }
    vec4 &sub(float f)       { sub3(f); w -= f; return *this; }
    vec4 &subw(float f)      { w -= f; return *this; }
    vec4 &neg3()             { x = -x; y = -y; z = -z; return *this; }
    vec4 &neg()              { neg3(); w = -w; return *this; }
    vec4 &clamp(float l, float h);

    template<class A, class B>
    vec4 &cross(const A &a, const B &b) { x = a.y*b.z-a.z*b.y; y = a.z*b.x-a.x*b.z; z = a.x*b.y-a.y*b.x; return *this; }
    vec4 &cross(const vec &o, const vec &a, const vec &b);

    void setxyz(const vec &v);

    vec4 &rotate_around_z(float c, float s) { float rx = x, ry = y; x = c*rx-s*ry; y = c*ry+s*rx; return *this; }
    vec4 &rotate_around_x(float c, float s) { float ry = y, rz = z; y = c*ry-s*rz; z = c*rz+s*ry; return *this; }
    vec4 &rotate_around_y(float c, float s) { float rx = x, rz = z; x = c*rx-s*rz; z = c*rz+s*rx; return *this; }

    vec4 &rotate_around_z(float angle);
    vec4 &rotate_around_x(float angle);
    vec4 &rotate_around_y(float angle);

    vec4 &rotate_around_z(const vec2 &sc);
    vec4 &rotate_around_x(const vec2 &sc);
    vec4 &rotate_around_y(const vec2 &sc);
};

void to_json(nlohmann::json& document, const vec4& v);
void from_json(const nlohmann::json& document, vec4& v);