#include "vec4.h"
#include "vec.h"
#include "vec2.h"
#include <nlohmann/json.hpp>
#include <math.h>
#include <algorithm>

vec4::vec4(const vec &p, float w) : x(p.x), y(p.y), z(p.z), w(w) {}
vec4::vec4(const vec2 &p, float z, float w) : x(p.x), y(p.y), z(z), w(w) {}

float vec4::dot3(const vec &o) const { return x*o.x + y*o.y + z*o.z; }

float vec4::dot(const vec &o) const  { return x*o.x + y*o.y + z*o.z + w; }

float vec4::magnitude() const  { return sqrtf(squaredlen()); }
float vec4::magnitude3() const { return sqrtf(dot3(*this)); }

vec4 &vec4::clamp(float l, float h) { x = std::clamp(x, l, h); y = std::clamp(y, l, h); z = std::clamp(z, l, h); w = std::clamp(w, l, h); return *this; }

vec4 &vec4::mul(const vec &o)  { x *= o.x; y *= o.y; z *= o.z; return *this; }
vec4 &vec4::div(const vec &o)  { x /= o.x; y /= o.y; z /= o.z; return *this; }
vec4 &vec4::add(const vec &o)  { x += o.x; y += o.y; z += o.z; return *this; }
vec4 &vec4::sub(const vec &o)  { x -= o.x; y -= o.y; z -= o.z; return *this; }
vec4 &vec4::cross(const vec &o, const vec &a, const vec &b) { return cross(vec(a).sub(o), vec(b).sub(o)); }
void vec4::setxyz(const vec &v) { x = v.x; y = v.y; z = v.z; }

vec4 &vec4::rotate_around_z(float angle) { return rotate_around_z(cosf(angle), sinf(angle)); }
vec4 &vec4::rotate_around_x(float angle) { return rotate_around_x(cosf(angle), sinf(angle)); }
vec4 &vec4::rotate_around_y(float angle) { return rotate_around_y(cosf(angle), sinf(angle)); }

vec4 &vec4::rotate_around_z(const vec2 &sc) { return rotate_around_z(sc.x, sc.y); }
vec4 &vec4::rotate_around_x(const vec2 &sc) { return rotate_around_x(sc.x, sc.y); }
vec4 &vec4::rotate_around_y(const vec2 &sc) { return rotate_around_y(sc.x, sc.y); }


void to_json(nlohmann::json& document, const vec4& v)
{
	document = nlohmann::json{{"x", v.x}, {"y", v.y}, {"z", v.z}, {"w", v.w}};
}

void from_json(const nlohmann::json& document, vec4& v) {
	document.at("x").get_to(v.x);
	document.at("y").get_to(v.y);
	document.at("z").get_to(v.z);
	document.at("w").get_to(v.w);
}

