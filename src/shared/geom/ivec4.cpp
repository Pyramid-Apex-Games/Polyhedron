#include "ivec4.h"
#include "ivec.h"
#include "vec4.h"

ivec4::ivec4(const ivec &p, int w) : x(p.x), y(p.y), z(p.z), w(w) {}

ivec4::ivec4(const vec4 &v) : x(int(v.x)), y(int(v.y)), z(int(v.z)), w(int(v.w)) {}
