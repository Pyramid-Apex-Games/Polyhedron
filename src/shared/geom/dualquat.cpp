#include "dualquat.h"
#include "matrix4x3.h"

dualquat::dualquat(const matrix4x3 &m) : real(m)
{
    dual.x =  0.5f*( m.d.x*real.w + m.d.y*real.z - m.d.z*real.y);
    dual.y =  0.5f*(-m.d.x*real.z + m.d.y*real.w + m.d.z*real.x);
    dual.z =  0.5f*( m.d.x*real.y - m.d.y*real.x + m.d.z*real.w);
    dual.w = -0.5f*( m.d.x*real.x + m.d.y*real.y + m.d.z*real.z);
}

dualquat::dualquat(const quat &q, const vec &p)
        : real(q),
          dual(0.5f*( p.x*q.w + p.y*q.z - p.z*q.y),
               0.5f*(-p.x*q.z + p.y*q.w + p.z*q.x),
               0.5f*( p.x*q.y - p.y*q.x + p.z*q.w),
               -0.5f*( p.x*q.x + p.y*q.y + p.z*q.z))
{
}

void dualquat::translate(const vec &p)
{
    dual.x +=  0.5f*( p.x*real.w + p.y*real.z - p.z*real.y);
    dual.y +=  0.5f*(-p.x*real.z + p.y*real.w + p.z*real.x);
    dual.z +=  0.5f*( p.x*real.y - p.y*real.x + p.z*real.w);
    dual.w += -0.5f*( p.x*real.x + p.y*real.y + p.z*real.z);
}

vec dualquat::transform(const vec &v) const
{
    return vec().cross(real, vec().cross(real, v).madd(v, real.w).add(vec(dual))).madd(vec(dual), real.w).msub(vec(real), dual.w).mul(2).add(v);
}

vec dualquat::transposedtransform(const vec &v) const
{
    return dualquat(*this).invert().transform(v);
}

vec dualquat::transformnormal(const vec &v) const
{
    return real.rotate(v);
}

vec dualquat::transposedtransformnormal(const vec &v) const
{
    return real.invertedrotate(v);
}

vec dualquat::gettranslation() const
{
    return vec().cross(real, dual).madd(vec(dual), real.w).msub(vec(real), dual.w).mul(2);
}
