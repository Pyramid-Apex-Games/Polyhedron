#include "vec.h"
#include "vec4.h"
#include "vec2.h"
#include "ivec.h"
#include "usvec.h"
#include "svec.h"
#include <nlohmann/json.hpp>
#include <math.h>
#include <algorithm>

vec::vec(const vec2 &v, float z) : x(v.x), y(v.y), z(z) {}

vec::vec(const vec4 &v) : x(v.x), y(v.y), z(v.z) {}
vec::vec(const ivec &v) : x(v.x), y(v.y), z(v.z) {}
vec::vec(const usvec &v) : x(v.x), y(v.y), z(v.z) {}
vec::vec(const svec &v) : x(v.x), y(v.y), z(v.z) {}

vec::vec(float yaw, float pitch) : x(-sinf(yaw)*cosf(pitch)), y(cosf(yaw)*cosf(pitch)), z(sinf(pitch)) {}

float vec::dot2(const vec2 &o) const { return x*o.x + y*o.y; }
float vec::absdot(const vec &o) const { return fabs(x*o.x) + fabs(y*o.y) + fabs(z*o.z); }

vec &vec::min(const vec &o)   { x = std::min(x, o.x); y = std::min(y, o.y); z = std::min(z, o.z); return *this; }
vec &vec::max(const vec &o)   { x = std::max(x, o.x); y = std::max(y, o.y); z = std::max(z, o.z); return *this; }
vec &vec::min(float f)        { x = std::min(x, f); y = std::min(y, f); z = std::min(z, f); return *this; }
vec &vec::max(float f)        { x = std::max(x, f); y = std::max(y, f); z = std::max(z, f); return *this; }
vec &vec::abs() { x = fabs(x); y = fabs(y); z = fabs(z); return *this; }
vec &vec::clamp(float l, float h) { x =std::clamp(x, l, h); y = std::clamp(y, l, h); z = std::clamp(z, l, h); return *this; }
float vec::magnitude2() const { return sqrtf(dot2(*this)); }
float vec::magnitude() const  { return sqrtf(squaredlen()); }
float vec::dist(const vec &e) const { return sqrtf(squaredist(e)); }
float vec::dist2(const vec &o) const { float dx = x-o.x, dy = y-o.y; return sqrtf(dx*dx + dy*dy); }
vec &vec::projectxy(const vec &n)
{
    float m = squaredlen(), k = dot(n);
    projectxydir(n);
    rescale(sqrtf(std::max(m - k*k, 0.0f)));
    return *this;
}
vec &vec::projectxy(const vec &n, float threshold)
{
    float m = squaredlen(), k = std::min(dot(n), threshold);
    projectxydir(n);
    rescale(sqrtf(std::max(m - k*k, 0.0f)));
    return *this;
}

vec &vec::rotate_around_z(float angle) { return rotate_around_z(cosf(angle), sinf(angle)); }
vec &vec::rotate_around_x(float angle) { return rotate_around_x(cosf(angle), sinf(angle)); }
vec &vec::rotate_around_y(float angle) { return rotate_around_y(cosf(angle), sinf(angle)); }

vec &vec::rotate_around_z(const vec2 &sc) { return rotate_around_z(sc.x, sc.y); }
vec &vec::rotate_around_x(const vec2 &sc) { return rotate_around_x(sc.x, sc.y); }
vec &vec::rotate_around_y(const vec2 &sc) { return rotate_around_y(sc.x, sc.y); }

vec &vec::rotate(float angle, const vec &d) { return rotate(cosf(angle), sinf(angle), d); }
vec &vec::rotate(const vec2 &sc, const vec &d) { return rotate(sc.x, sc.y, d); }

void vec::orthogonal(const vec &d)
{
    *this = fabs(d.x) > fabs(d.z) ? vec(-d.y, d.x, 0) : vec(0, -d.z, d.y);
}
int vec::tohexcolor() const { return (int(std::clamp(r, 0.0f, 1.0f)*255)<<16)|(int(std::clamp(g, 0.0f, 1.0f)*255)<<8)|int(std::clamp(b, 0.0f, 1.0f)*255); }


void to_json(nlohmann::json& document, const vec& v)
{
	document = nlohmann::json{{"x", v.x}, {"y", v.y}, {"z", v.z}};
}

void from_json(const nlohmann::json& document, vec& v) {
	document.at("x").get_to(v.x);
	document.at("y").get_to(v.y);
	document.at("z").get_to(v.z);
}
