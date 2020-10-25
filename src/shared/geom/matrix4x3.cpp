#include "matrix4x3.h"
#include "matrix4.h"
#include "matrix3.h"
#include "dualquat.h"
#include "vec2.h"

matrix4x3::matrix4x3(const matrix4 &m)
        : a(m.a), b(m.b), c(m.c), d(m.d)
{}
matrix4x3::matrix4x3(const matrix3 &rot, const vec &trans) : a(rot.a), b(rot.b), c(rot.c), d(trans) {}
matrix4x3::matrix4x3(const dualquat &dq)
{
    vec4 r = vec4(dq.real).mul(1/dq.real.squaredlen()), rr = vec4(r).mul(dq.real);
    r.mul(2);
    float xy = r.x*dq.real.y, xz = r.x*dq.real.z, yz = r.y*dq.real.z,
            wx = r.w*dq.real.x, wy = r.w*dq.real.y, wz = r.w*dq.real.z;
    a = vec(rr.w + rr.x - rr.y - rr.z, xy + wz, xz - wy);
    b = vec(xy - wz, rr.w + rr.y - rr.x - rr.z, yz + wx);
    c = vec(xz + wy, yz - wx, rr.w + rr.z - rr.x - rr.y);
    d = vec(-(dq.dual.w*r.x - dq.dual.x*r.w + dq.dual.y*r.z - dq.dual.z*r.y),
            -(dq.dual.w*r.y - dq.dual.x*r.z - dq.dual.y*r.w + dq.dual.z*r.x),
            -(dq.dual.w*r.z + dq.dual.x*r.y - dq.dual.y*r.x - dq.dual.z*r.w));

}

void matrix4x3::mul(const matrix3 &m, const matrix4x3 &n)
{
    a = vec(m.a).mul(n.a.x).madd(m.b, n.a.y).madd(m.c, n.a.z);
    b = vec(m.a).mul(n.b.x).madd(m.b, n.b.y).madd(m.c, n.b.z);
    c = vec(m.a).mul(n.c.x).madd(m.b, n.c.y).madd(m.c, n.c.z);
    d = vec(m.a).mul(n.d.x).madd(m.b, n.d.y).madd(m.c, n.d.z);
}

void matrix4x3::rotate(float ck, float sk, const vec &axis)
{
    matrix3 m;
    m.rotate(ck, sk, axis);
    *this = matrix4x3(m, vec(0, 0, 0));
}

void matrix4x3::rotate_around_x(float angle) { rotate_around_x(cosf(angle), sinf(angle)); }
void matrix4x3::rotate_around_x(const vec2 &sc) { rotate_around_x(sc.x, sc.y); }

void matrix4x3::rotate_around_y(float angle) { rotate_around_y(cosf(angle), sinf(angle)); }
void matrix4x3::rotate_around_y(const vec2 &sc) { rotate_around_y(sc.x, sc.y); }

void matrix4x3::rotate_around_z(float angle) { rotate_around_z(cosf(angle), sinf(angle)); }
void matrix4x3::rotate_around_z(const vec2 &sc) { rotate_around_z(sc.x, sc.y); }

vec matrix4x3::transform(const vec2 &o) const { return vec(d).madd(a, o.x).madd(b, o.y); }

vec4 matrix4x3::rowx() const { return vec4(a.x, b.x, c.x, d.x); }
vec4 matrix4x3::rowy() const { return vec4(a.y, b.y, c.y, d.y); }
vec4 matrix4x3::rowz() const { return vec4(a.z, b.z, c.z, d.z); }
