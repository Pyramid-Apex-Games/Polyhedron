#include "quat.h"
#include "vec.h"
#include "matrix4x3.h"
#ifndef M_PI
#define M_PI          0x1.921fb54442d18p+1
#endif

quat::quat(float x, float y, float z, float w) : vec4(x, y, z, w) {}

quat::quat(const vec &axis, float angle)
{
    w = cosf(angle/2);
    float s = sinf(angle/2);
    x = s*axis.x;
    y = s*axis.y;
    z = s*axis.z;
}

quat::quat(const vec &u, const vec &v)
{
    w = sqrtf(u.squaredlen() * v.squaredlen()) + u.dot(v);
    cross(u, v);
    normalize();
}

quat::quat(const vec &v)
{
    x = v.x;
    y = v.y;
    z = v.z;
    restorew();
}

void quat::restorew() { w = 1.0f-x*x-y*y-z*z; w = w<0 ? 0 : -sqrtf(w); }

quat &quat::add(const vec4 &o) { vec4::add(o); return *this; }
quat &quat::sub(const vec4 &o) { vec4::sub(o); return *this; }
quat &quat::mul(float k) { vec4::mul(k); return *this; }

void quat::calcangleaxis(float &angle, vec &axis) const
{
    float rr = dot3(*this);
    if(rr>0)
    {
        angle = 2*acosf(w);
        axis = vec(x, y, z).mul(1/rr);
    }
    else { angle = 0; axis = vec(0, 0, 1); }
}

vec quat::calcangles() const
{
    vec4 qq = vec4(*this).square();
    float rr = qq.x + qq.y + qq.z + qq.w,
            t = x*y + z*w;
    if(fabs(t) > 0.49999f*rr) return t < 0 ? vec(-2*atan2f(x, w), -M_PI/2, 0) : vec(2*atan2f(x, w), M_PI/2, 0);
    return vec(atan2f(2*(y*w - x*z), qq.x - qq.y - qq.z + qq.w),
               asinf(2*t/rr),
               atan2f(2*(x*w - y*z), -qq.x + qq.y - qq.z + qq.w));
}

vec quat::rotate(const vec &v) const
{
    return vec().cross(*this, vec().cross(*this, v).madd(v, w)).mul(2).add(v);
}

vec quat::invertedrotate(const vec &v) const
{
    return vec().cross(*this, vec().cross(*this, v).msub(v, w)).mul(2).add(v);
}

