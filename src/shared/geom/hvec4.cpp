#include "hvec4.h"
#include "vec.h"
#include "vec4.h"

hvec4::hvec4(const vec &v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}
hvec4::hvec4(const vec4 &v) : x(v.x), y(v.y), z(v.z), w(v.w) {}