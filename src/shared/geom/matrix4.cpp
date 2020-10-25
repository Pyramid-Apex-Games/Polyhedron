#include "matrix4.h"
#include "matrix3.h"
#include "matrix4x3.h"
#include "vec2.h"
#include "dvec4.h"
#include "plane.h"
#include "constants.h"
#include <math.h>

matrix4::matrix4(const matrix4x3 &m)
        : a(m.a, 0), b(m.b, 0), c(m.c, 0), d(m.d, 1)
{}
matrix4::matrix4(const matrix3 &rot, const vec &trans)
        : a(rot.a, 0), b(rot.b, 0), c(rot.c, 0), d(trans, 1)
{}

void matrix4::mul(const matrix4 &x, const matrix3 &y)
{
    a = vec4(x.a).mul(y.a.x).madd(x.b, y.a.y).madd(x.c, y.a.z);
    b = vec4(x.a).mul(y.b.x).madd(x.b, y.b.y).madd(x.c, y.b.z);
    c = vec4(x.a).mul(y.c.x).madd(x.b, y.c.y).madd(x.c, y.c.z);
    d = x.d;
}

void matrix4::mul(const matrix3 &y) { mul(matrix4(*this), y); }

void matrix4::muld(const matrix4 &x, const matrix4 &y) { mult<dvec4>(x, y); }
void matrix4::muld(const matrix4 &y) { mult<dvec4>(matrix4(*this), y); }

void matrix4::rotate_around_x(float angle) { rotate_around_x(cosf(angle), sinf(angle)); }
void matrix4::rotate_around_x(const vec2 &sc) { rotate_around_x(sc.x, sc.y); }

void matrix4::rotate_around_y(float angle) { rotate_around_y(cosf(angle), sinf(angle)); }
void matrix4::rotate_around_y(const vec2 &sc) { rotate_around_y(sc.x, sc.y); }

void matrix4::rotate_around_z(float angle) { rotate_around_z(cosf(angle), sinf(angle)); }
void matrix4::rotate_around_z(const vec2 &sc) { rotate_around_z(sc.x, sc.y); }

void matrix4::rotate(float ck, float sk, const vec &axis)
{
    matrix3 m;
    m.rotate(ck, sk, axis);
    mul(m);
}
void matrix4::rotate(float angle, const vec &dir) { rotate(cosf(angle), sinf(angle), dir); }
void matrix4::rotate(const vec2 &sc, const vec &dir) { rotate(sc.x, sc.y, dir); }

void matrix4::transpose()
{
    std::swap(a.y, b.x); std::swap(a.z, c.x); std::swap(a.w, d.x);
    std::swap(b.z, c.y); std::swap(b.w, d.y);
    std::swap(c.w, d.z);
}

void matrix4::clip(const plane &p, const matrix4 &m)
{
    float x = ((p.x<0 ? -1 : (p.x>0 ? 1 : 0)) + m.c.x) / m.a.x,
            y = ((p.y<0 ? -1 : (p.y>0 ? 1 : 0)) + m.c.y) / m.b.y,
            w = (1 + m.c.z) / m.d.z,
            scale = 2 / (x*p.x + y*p.y - p.z + w*p.offset);
    a = vec4(m.a.x, m.a.y, p.x*scale, m.a.w);
    b = vec4(m.b.x, m.b.y, p.y*scale, m.b.w);
    c = vec4(m.c.x, m.c.y, p.z*scale + 1.0f, m.c.w);
    d = vec4(m.d.x, m.d.y, p.offset*scale, m.d.w);
}

void matrix4::transposedtransform(const plane &in, plane &out) const
{
    out.x = in.dist(a);
    out.y = in.dist(b);
    out.z = in.dist(c);
    out.offset = in.dist(d);
}

float matrix4::getscale() const
{
    return sqrtf(a.x*a.y + b.x*b.x + c.x*c.x);
}

vec2 matrix4::lineardepthscale() const
{
    return vec2(d.w, -d.z).div(c.z*d.w - d.z*c.w);
}

void matrix4::perspective(float fovy, float aspect, float znear, float zfar)
{
    float ydist = znear * tan(fovy/2*RAD), xdist = ydist * aspect;
    frustum(-xdist, xdist, -ydist, ydist, znear, zfar);
}
