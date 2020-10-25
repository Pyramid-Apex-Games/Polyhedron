#pragma once
#include "vec.h"

struct quat;
struct matrix4x3;
struct matrix4;
struct vec2;

struct matrix3
{
    vec a, b, c;

    matrix3() {}
    matrix3(const vec &a, const vec &b, const vec &c) : a(a), b(b), c(c) {}
    explicit matrix3(float angle, const vec &axis) { rotate(angle, axis); }
    explicit matrix3(const quat &q);
    explicit matrix3(const matrix4x3 &m);
    explicit matrix3(const matrix4 &m);

    void mul(const matrix3 &m, const matrix3 &n)
    {
        a = vec(m.a).mul(n.a.x).madd(m.b, n.a.y).madd(m.c, n.a.z);
        b = vec(m.a).mul(n.b.x).madd(m.b, n.b.y).madd(m.c, n.b.z);
        c = vec(m.a).mul(n.c.x).madd(m.b, n.c.y).madd(m.c, n.c.z);
    }
    void mul(const matrix3 &n) { mul(matrix3(*this), n); }

    void multranspose(const matrix3 &m, const matrix3 &n)
    {
        a = vec(m.a).mul(n.a.x).madd(m.b, n.b.x).madd(m.c, n.c.x);
        b = vec(m.a).mul(n.a.y).madd(m.b, m.b.y).madd(m.c, n.c.y);
        c = vec(m.a).mul(n.a.z).madd(m.b, n.b.z).madd(m.c, n.c.z);
    }
    void multranspose(const matrix3 &n) { multranspose(matrix3(*this), n); }

    void transposemul(const matrix3 &m, const matrix3 &n)
    {
        a = vec(m.a.dot(n.a), m.b.dot(n.a), m.c.dot(n.a));
        b = vec(m.a.dot(n.b), m.b.dot(n.b), m.c.dot(n.b));
        c = vec(m.a.dot(n.c), m.b.dot(n.c), m.c.dot(n.c));
    }
    void transposemul(const matrix3 &n) { transposemul(matrix3(*this), n); }

    void transpose();

    template<class M>
    void transpose(const M &m)
    {
        a = vec(m.a.x, m.b.x, m.c.x);
        b = vec(m.a.y, m.b.y, m.c.y);
        c = vec(m.a.z, m.b.z, m.c.z);
    }

    void invert(const matrix3 &o)
    {
        vec unscale(1/o.a.squaredlen(), 1/o.b.squaredlen(), 1/o.c.squaredlen());
        transpose(o);
        a.mul(unscale);
        b.mul(unscale);
        c.mul(unscale);
    }
    void invert() { invert(matrix3(*this)); }

    void normalize()
    {
        a.normalize();
        b.normalize();
        c.normalize();
    }

    void scale(float k)
    {
        a.mul(k);
        b.mul(k);
        c.mul(k);
    }

    void rotate(float angle, const vec &axis);

    void rotate(float ck, float sk, const vec &axis)
    {
        a = vec(axis.x*axis.x*(1-ck)+ck, axis.x*axis.y*(1-ck)+axis.z*sk, axis.x*axis.z*(1-ck)-axis.y*sk);
        b = vec(axis.x*axis.y*(1-ck)-axis.z*sk, axis.y*axis.y*(1-ck)+ck, axis.y*axis.z*(1-ck)+axis.x*sk);
        c = vec(axis.x*axis.z*(1-ck)+axis.y*sk, axis.y*axis.z*(1-ck)-axis.x*sk, axis.z*axis.z*(1-ck)+ck);
    }

    void setyaw(float ck, float sk)
    {
        a = vec(ck, sk, 0);
        b = vec(-sk, ck, 0);
        c = vec(0, 0, 1);
    }

    void setyaw(float angle);

    float trace() const { return a.x + b.y + c.z; }

    bool calcangleaxis(float tr, float &angle, vec &axis, float threshold = 1e-16f) const;

    bool calcangleaxis(float &angle, vec &axis, float threshold = 1e-16f) const { return calcangleaxis(trace(), angle, axis, threshold); }

    vec transform(const vec &o) const
    {
        return vec(a).mul(o.x).madd(b, o.y).madd(c, o.z);
    }
    vec transposedtransform(const vec &o) const { return vec(a.dot(o), b.dot(o), c.dot(o)); }
    vec abstransform(const vec &o) const
    {
        return vec(a).mul(o.x).abs().add(vec(b).mul(o.y).abs()).add(vec(c).mul(o.z).abs());
    }
    vec abstransposedtransform(const vec &o) const
    {
        return vec(a.absdot(o), b.absdot(o), c.absdot(o));
    }

    void identity()
    {
        a = vec(1, 0, 0);
        b = vec(0, 1, 0);
        c = vec(0, 0, 1);
    }

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

    vec transform(const vec2 &o);
    vec transposedtransform(const vec2 &o) const;

    vec rowx() const { return vec(a.x, b.x, c.x); }
    vec rowy() const { return vec(a.y, b.y, c.y); }
    vec rowz() const { return vec(a.z, b.z, c.z); }
};
