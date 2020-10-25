#pragma once
#include "vec.h"

struct matrix3;
struct dualquat;
struct matrix4;

struct matrix4x3
{
    vec a, b, c, d;

    matrix4x3() {}
    matrix4x3(const vec &a, const vec &b, const vec &c, const vec &d) : a(a), b(b), c(c), d(d) {}
    matrix4x3(const matrix3 &rot, const vec &trans);
    matrix4x3(const dualquat &dq);
    explicit matrix4x3(const matrix4 &m);

    void mul(float k)
    {
        a.mul(k);
        b.mul(k);
        c.mul(k);
        d.mul(k);
    }

    void setscale(float x, float y, float z) { a.x = x; b.y = y; c.z = z; }
    void setscale(const vec &v) { setscale(v.x, v.y, v.z); }
    void setscale(float n) { setscale(n, n, n); }

    void scale(float x, float y, float z)
    {
        a.mul(x);
        b.mul(y);
        c.mul(z);
    }
    void scale(const vec &v) { scale(v.x, v.y, v.z); }
    void scale(float n) { scale(n, n, n); }

    void settranslation(const vec &p) { d = p; }
    void settranslation(float x, float y, float z) { d = vec(x, y, z); }

    void translate(const vec &p) { d.madd(a, p.x).madd(b, p.y).madd(c, p.z); }
    void translate(float x, float y, float z) { translate(vec(x, y, z)); }
    void translate(const vec &p, float scale) { translate(vec(p).mul(scale)); }

    void accumulate(const matrix4x3 &m, float k)
    {
        a.madd(m.a, k);
        b.madd(m.b, k);
        c.madd(m.c, k);
        d.madd(m.d, k);
    }

    void normalize()
    {
        a.normalize();
        b.normalize();
        c.normalize();
    }

    void lerp(const matrix4x3 &to, float t)
    {
        a.lerp(to.a, t);
        b.lerp(to.b, t);
        c.lerp(to.c, t);
        d.lerp(to.d, t);
    }
    void lerp(const matrix4x3 &from, const matrix4x3 &to, float t)
    {
        a.lerp(from.a, to.a, t);
        b.lerp(from.b, to.b, t);
        c.lerp(from.c, to.c, t);
        d.lerp(from.d, to.d, t);
    }

    void identity()
    {
        a = vec(1, 0, 0);
        b = vec(0, 1, 0);
        c = vec(0, 0, 1);
        d = vec(0, 0, 0);
    }

    void mul(const matrix4x3 &m, const matrix4x3 &n)
    {
        a = vec(m.a).mul(n.a.x).madd(m.b, n.a.y).madd(m.c, n.a.z);
        b = vec(m.a).mul(n.b.x).madd(m.b, n.b.y).madd(m.c, n.b.z);
        c = vec(m.a).mul(n.c.x).madd(m.b, n.c.y).madd(m.c, n.c.z);
        d = vec(m.d).madd(m.a, n.d.x).madd(m.b, n.d.y).madd(m.c, n.d.z);
    }
    void mul(const matrix4x3 &n) { mul(matrix4x3(*this), n); }

    void mul(const matrix3 &m, const matrix4x3 &n);

    void mul(const matrix3 &rot, const vec &trans, const matrix4x3 &n)
    {
        mul(rot, n);
        d.add(trans);
    }

    void transpose()
    {
        d = vec(a.dot(d), b.dot(d), c.dot(d)).neg();
        std::swap(a.y, b.x); std::swap(a.z, c.x);
        std::swap(b.z, c.y);
    }

    void transpose(const matrix4x3 &o)
    {
        a = vec(o.a.x, o.b.x, o.c.x);
        b = vec(o.a.y, o.b.y, o.c.y);
        c = vec(o.a.z, o.b.z, o.c.z);
        d = vec(o.a.dot(o.d), o.b.dot(o.d), o.c.dot(o.d)).neg();
    }

    void transposemul(const matrix4x3 &m, const matrix4x3 &n)
    {
        vec t(m.a.dot(m.d), m.b.dot(m.d), m.c.dot(m.d));
        a = vec(m.a.dot(n.a), m.b.dot(n.a), m.c.dot(n.a));
        b = vec(m.a.dot(n.b), m.b.dot(n.b), m.c.dot(n.b));
        c = vec(m.a.dot(n.c), m.b.dot(n.c), m.c.dot(n.c));
        d = vec(m.a.dot(n.d), m.b.dot(n.d), m.c.dot(n.d)).sub(t);
    }

    void multranspose(const matrix4x3 &m, const matrix4x3 &n)
    {
        vec t(n.a.dot(n.d), n.b.dot(n.d), n.c.dot(n.d));
        a = vec(m.a).mul(n.a.x).madd(m.b, n.b.x).madd(m.c, n.c.x);
        b = vec(m.a).mul(n.a.y).madd(m.b, m.b.y).madd(m.c, n.c.y);
        c = vec(m.a).mul(n.a.z).madd(m.b, n.b.z).madd(m.c, n.c.z);
        d = vec(m.d).msub(m.a, t.x).msub(m.b, t.y).msub(m.c, t.z);
    }

    void invert(const matrix4x3 &o)
    {
        vec unscale(1/o.a.squaredlen(), 1/o.b.squaredlen(), 1/o.c.squaredlen());
        transpose(o);
        a.mul(unscale);
        b.mul(unscale);
        c.mul(unscale);
        d.mul(unscale);
    }
    void invert() { invert(matrix4x3(*this)); }

    void rotate(float angle, const vec &d)
    {
        rotate(cosf(angle), sinf(angle), d);
    }

    void rotate(float ck, float sk, const vec &axis);

    void rotate_around_x(float ck, float sk)
    {
        vec rb = vec(b).mul(ck).madd(c, sk),
                rc = vec(c).mul(ck).msub(b, sk);
        b = rb;
        c = rc;
    }
    void rotate_around_x(float angle);
    void rotate_around_x(const vec2 &sc);

    void rotate_around_y(float ck, float sk)
    {
        vec rc = vec(c).mul(ck).madd(a, sk),
                ra = vec(a).mul(ck).msub(c, sk);
        c = rc;
        a = ra;
    }
    void rotate_around_y(float angle);
    void rotate_around_y(const vec2 &sc);

    void rotate_around_z(float ck, float sk)
    {
        vec ra = vec(a).mul(ck).madd(b, sk),
                rb = vec(b).mul(ck).msub(a, sk);
        a = ra;
        b = rb;
    }
    void rotate_around_z(float angle);
    void rotate_around_z(const vec2 &sc);

    vec transform(const vec &o) const { return vec(d).madd(a, o.x).madd(b, o.y).madd(c, o.z); }
    vec transposedtransform(const vec &o) const { vec p = vec(o).sub(d); return vec(a.dot(p), b.dot(p), c.dot(p)); }
    vec transformnormal(const vec &o) const { return vec(a).mul(o.x).madd(b, o.y).madd(c, o.z); }
    vec transposedtransformnormal(const vec &o) const { return vec(a.dot(o), b.dot(o), c.dot(o)); }
    vec transform(const vec2 &o) const;

    vec4 rowx() const;
    vec4 rowy() const;
    vec4 rowz() const;
};
