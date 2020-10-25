#include "ivec2.h"
#include "vec2.h"
#include "ivec.h"
#include <math.h>
#include <algorithm>

ivec2::ivec2(const vec2 &v) : x(int(v.x)), y(int(v.y)) {}
ivec2::ivec2(const ivec &v) : x(v.x), y(v.y) {}

ivec2 &ivec2::min(const ivec2 &o) { x = std::min(x, o.x); y = std::min(y, o.y); return *this; }
ivec2 &ivec2::max(const ivec2 &o) { x = std::max(x, o.x); y = std::max(y, o.y); return *this; }
ivec2 &ivec2::min(int n) { x = std::min(x, n); y = std::min(y, n); return *this; }
ivec2 &ivec2::max(int n) { x = std::max(x, n); y = std::max(y, n); return *this; }
ivec2 &ivec2::abs() { x = ::abs(x); y = ::abs(y); return *this; }
