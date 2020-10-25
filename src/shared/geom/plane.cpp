#include "plane.h"
#include "vec4.h"

float plane::dist(const vec4 &p) const { return p.dot3(*this) + p.w*offset; }

plane::plane(const vec4 &p) : vec(p), offset(p.w) {}
