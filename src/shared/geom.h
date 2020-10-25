#pragma once

#include "geom/bvec.h"
#include "geom/bvec4.h"
#include "geom/dualquat.h"
#include "geom/dvec4.h"
#include "geom/half.h"
#include "geom/hvec.h"
#include "geom/hvec2.h"
#include "geom/hvec4.h"
#include "geom/ivec.h"
#include "geom/ivec2.h"
#include "geom/ivec4.h"
#include "geom/matrix.h"
#include "geom/matrix2.h"
#include "geom/matrix3.h"
#include "geom/matrix4.h"
#include "geom/matrix4x3.h"
#include "geom/plane.h"
#include "geom/quat.h"
#include "geom/squat.h"
#include "geom/svec.h"
#include "geom/triangle.h"
#include "geom/usvec.h"
#include "geom/vec.h"
#include "geom/vec2.h"
#include "geom/vec4.h"


bool raysphereintersect(const vec &center, float radius, const vec &o, const vec &ray, float &dist);
bool rayboxintersect(const vec &b, const vec &s, const vec &o, const vec &ray, float &dist, int &orient);
bool linecylinderintersect(const vec &from, const vec &to, const vec &start, const vec &end, float radius, float &dist);
int polyclip(const vec *in, int numin, const vec &dir, float below, float above, vec *out);

extern const vec2 sincos360[];
static inline int mod360(int angle)
{
    if(angle < 0) angle = 360 + (angle <= -360 ? angle%360 : angle);
    else if(angle >= 360) angle %= 360;
    return angle;
}
static inline const vec2 &sincosmod360(int angle) { return sincos360[mod360(angle)]; }
static inline float cos360(int angle) { return sincos360[angle].x; }
static inline float sin360(int angle) { return sincos360[angle].y; }
static inline float tan360(int angle) { const vec2 &sc = sincos360[angle]; return sc.y/sc.x; }
static inline float cotan360(int angle) { const vec2 &sc = sincos360[angle]; return sc.x/sc.y; }

