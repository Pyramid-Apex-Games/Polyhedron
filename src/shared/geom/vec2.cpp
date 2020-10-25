#include "vec2.h"
#include "tools.h"
#include <math.h>
#include <algorithm>
#include "vec.h"
#include "vec4.h"

float & vec2::operator[](int i)
{
     return v[i]; 
}
float vec2::operator[](int i) const
{
     return v[i]; 
}
bool vec2::operator==(const vec2 & o) const
{
     return x == o.x && y == o.y; 
}
bool vec2::operator!=(const vec2 & o) const
{
     return x != o.x || y != o.y; 
}
bool vec2::iszero() const
{
     return x==0 && y==0; 
}
float vec2::dot(const vec2 & o) const
{
     return x*o.x + y*o.y; 
}
float vec2::squaredlen() const
{
     return dot(*this); 
}
float vec2::magnitude() const
{
     return sqrtf(squaredlen()); 
}
vec2 & vec2::normalize()
{
     mul(1/magnitude()); return *this; 
}
vec2 & vec2::safenormalize()
{
     float m = magnitude(); if(m) mul(1/m); return *this; 
}
float vec2::cross(const vec2 & o) const
{
     return x*o.y - y*o.x; 
}
float vec2::squaredist(const vec2 & e) const
{
     return vec2(*this).sub(e).squaredlen(); 
}
float vec2::dist(const vec2 & e) const
{
     return sqrtf(squaredist(e)); 
}
vec2 & vec2::mul(float f)
{
     x *= f; y *= f; return *this; 
}
vec2 & vec2::mul(const vec2 & o)
{
     x *= o.x; y *= o.y; return *this; 
}
vec2 & vec2::square()
{
     mul(*this); return *this; 
}
vec2 & vec2::div(float f)
{
     x /= f; y /= f; return *this; 
}
vec2 & vec2::div(const vec2 & o)
{
     x /= o.x; y /= o.y; return *this; 
}
vec2 & vec2::recip()
{
     x = 1/x; y = 1/y; return *this; 
}
vec2 & vec2::add(float f)
{
     x += f; y += f; return *this; 
}
vec2 & vec2::add(const vec2 & o)
{
     x += o.x; y += o.y; return *this; 
}
vec2 & vec2::sub(float f)
{
     x -= f; y -= f; return *this; 
}
vec2 & vec2::sub(const vec2 & o)
{
     x -= o.x; y -= o.y; return *this; 
}
vec2 & vec2::neg()
{
     x = -x; y = -y; return *this; 
}
vec2 & vec2::min(const vec2 & o)
{
     x = std::min(x, o.x); y = std::min(y, o.y); return *this; 
}
vec2 & vec2::max(const vec2 & o)
{
     x = std::max(x, o.x); y = std::max(y, o.y); return *this; 
}
vec2 & vec2::min(float f)
{
     x = std::min(x, f); y = std::min(y, f); return *this; 
}
vec2 & vec2::max(float f)
{
     x = std::max(x, f); y = std::max(y, f); return *this; 
}
vec2 & vec2::abs()
{
     x = fabs(x); y = fabs(y); return *this; 
}
vec2 & vec2::clamp(float l, float h)
{
     x = ::clamp(x, l, h); y = ::clamp(y, l, h); return *this; 
}
vec2 & vec2::reflect(const vec2 & n)
{
     float k = 2*dot(n); x -= k*n.x; y -= k*n.y; return *this; 
}
vec2 & vec2::lerp(const vec2 & b, float t)
{
     x += (b.x-x)*t; y += (b.y-y)*t; return *this; 
}
vec2 & vec2::lerp(const vec2 & a, const vec2 & b, float t)
{
     x = a.x + (b.x-a.x)*t; y = a.y + (b.y-a.y)*t; return *this; 
}
vec2 & vec2::avg(const vec2 & b)
{
     add(b); mul(0.5f); return *this; 
}
vec2 & vec2::rotate_around_z(float c, float s)
{
     float rx = x, ry = y; x = c*rx-s*ry; y = c*ry+s*rx; return *this; 
}
vec2 & vec2::rotate_around_z(float angle)
{
     return rotate_around_z(cosf(angle), sinf(angle)); 
}
vec2 & vec2::rotate_around_z(const vec2 & sc)
{
     return rotate_around_z(sc.x, sc.y); 
}

vec2::vec2(const vec &v) : x(v.x), y(v.y) {}
vec2::vec2(const vec4 &v) : x(v.x), y(v.y) {}
