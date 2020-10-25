#include "ivec.h"
#include "ivec2.h"
#include "ivec4.h"
#include "usvec.h"
#include "svec.h"
#include "vec.h"
#include "plane.h"
#include "constants.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <math.h>

ivec::ivec(const ivec2 &v, int z) : x(v.x), y(v.y), z(z) {}
ivec::ivec(const ivec4 &v) : x(v.x), y(v.y), z(v.z) {}
ivec::ivec(const usvec &v) : x(v.x), y(v.y), z(v.z) {}
ivec::ivec(const svec &v) : x(v.x), y(v.y), z(v.z) {}

ivec::ivec(const vec &v) : x(int(v.x)), y(int(v.y)), z(int(v.z)) {}
ivec::ivec(int d, int row, int col, int depth)
{
    v[R[d]] = row;
    v[C[d]] = col;
    v[D[d]] = depth;
}

ivec &ivec::min(const ivec &o) { x = std::min(x, o.x); y = std::min(y, o.y); z = std::min(z, o.z); return *this; }
ivec &ivec::max(const ivec &o) { x = std::max(x, o.x); y = std::max(y, o.y); z = std::max(z, o.z); return *this; }
ivec &ivec::min(int n) { x = std::min(x, n); y = std::min(y, n); z = std::min(z, n); return *this; }
ivec &ivec::max(int n) { x = std::max(x, n); y = std::max(y, n); z = std::max(z, n); return *this; }
ivec &ivec::abs() { x = ::abs(x); y = ::abs(y); z = ::abs(z); return *this; }
ivec &ivec::clamp(int l, int h) { x = std::clamp(x, l, h); y = std::clamp(y, l, h); z = std::clamp(z, l, h); return *this; }

float ivec::dist(const plane &p) const { return x*p.x + y*p.y + z*p.z + p.offset; }

ivec ivec::floor(const vec &o) { return ivec(int(::floor(o.x)), int(::floor(o.y)), int(::floor(o.z))); }
ivec ivec::ceil(const vec &o) { return ivec(int(::ceil(o.x)), int(::ceil(o.y)), int(::ceil(o.z))); }


void to_json(nlohmann::json& document, const ivec& v)
{
	document = nlohmann::json{{"x", v.x}, {"y", v.y}, {"z", v.z}};
}

void from_json(const nlohmann::json& document, ivec& v) {
	document.at("x").get_to(v.x);
	document.at("y").get_to(v.y);
	document.at("z").get_to(v.z);
}
