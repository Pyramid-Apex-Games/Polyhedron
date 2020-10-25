#pragma once
#include "vec4.h"
#include "vec.h"

struct matrix3;
struct matrix4x3;
struct vec2;
struct plane;

struct matrix4
{
    vec4 a, b, c, d;

    matrix4() {}
    matrix4(const float *m) : a(m), b(m+4), c(m+8), d(m+12) {}
    matrix4(const vec &a, const vec &b, const vec &c = vec(0, 0, 1))
            : a(a.x, b.x, c.x, 0), b(a.y, b.y, c.y, 0), c(a.z, b.z, c.z, 0), d(0, 0, 0, 1)
    {}
    matrix4(const vec4 &a, const vec4 &b, const vec4 &c, const vec4 &d = vec4(0, 0, 0, 1))
            : a(a), b(b), c(c), d(d)
    {}
    matrix4(const matrix4x3 &m);
    matrix4(const matrix3 &rot, const vec &trans);

    void mul(const matrix4 &x, const matrix3 &y);
    void mul(const matrix3 &y);

    template<class T> void mult(const matrix4 &x, const matrix4 &y)
    {
        a = T(x.a).mul(y.a.x).madd(x.b, y.a.y).madd(x.c, y.a.z).madd(x.d, y.a.w);
        b = T(x.a).mul(y.b.x).madd(x.b, y.b.y).madd(x.c, y.b.z).madd(x.d, y.b.w);
        c = T(x.a).mul(y.c.x).madd(x.b, y.c.y).madd(x.c, y.c.z).madd(x.d, y.c.w);
        d = T(x.a).mul(y.d.x).madd(x.b, y.d.y).madd(x.c, y.d.z).madd(x.d, y.d.w);
    }

    void mul(const matrix4 &x, const matrix4 &y) { mult<vec4>(x, y); }
    void mul(const matrix4 &y) { mult<vec4>(matrix4(*this), y); }

    void muld(const matrix4 &x, const matrix4 &y);
    void muld(const matrix4 &y);

    void rotate_around_x(float ck, float sk)
    {
        vec4 rb = vec4(b).mul(ck).madd(c, sk),
                rc = vec4(c).mul(ck).msub(b, sk);
        b = rb;
        c = rc;
    }
    void rotate_around_x(float angle);
    void rotate_around_x(const vec2 &sc);

    void rotate_around_y(float ck, float sk)
    {
        vec4 rc = vec4(c).mul(ck).madd(a, sk),
                ra = vec4(a).mul(ck).msub(c, sk);
        c = rc;
        a = ra;
    }
    void rotate_around_y(float angle);
    void rotate_around_y(const vec2 &sc);

    void rotate_around_z(float ck, float sk)
    {
        vec4 ra = vec4(a).mul(ck).madd(b, sk),
                rb = vec4(b).mul(ck).msub(a, sk);
        a = ra;
        b = rb;
    }
    void rotate_around_z(float angle);
    void rotate_around_z(const vec2 &sc);

    void rotate(float ck, float sk, const vec &axis);
    void rotate(float angle, const vec &dir);
    void rotate(const vec2 &sc, const vec &dir);

    void identity()
    {
        a = vec4(1, 0, 0, 0);
        b = vec4(0, 1, 0, 0);
        c = vec4(0, 0, 1, 0);
        d = vec4(0, 0, 0, 1);
    }

    void settranslation(const vec &v) { d.setxyz(v); }
    void settranslation(float x, float y, float z) { d.x = x; d.y = y; d.z = z; }

    void translate(const vec &p) { d.madd(a, p.x).madd(b, p.y).madd(c, p.z); }
    void translate(float x, float y, float z) { translate(vec(x, y, z)); }
    void translate(const vec &p, float scale) { translate(vec(p).mul(scale)); }

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

    void scalexy(float x, float y)
    {
        a.x *= x; a.y *= y;
        b.x *= x; b.y *= y;
        c.x *= x; c.y *= y;
        d.x *= x; d.y *= y;
    }

    void scalez(float k)
    {
        a.z *= k;
        b.z *= k;
        c.z *= k;
        d.z *= k;
    }

    void reflectz(float z)
    {
        d.add(vec4(c).mul(2*z));
        c.neg();
    }

    void jitter(float x, float y)
    {
        a.x += x * a.w;
        a.y += y * a.w;
        b.x += x * b.w;
        b.y += y * b.w;
        c.x += x * c.w;
        c.y += y * c.w;
        d.x += x * d.w;
        d.y += y * d.w;
    }

    void transpose();

    void transpose(const matrix4 &m)
    {
        a = vec4(m.a.x, m.b.x, m.c.x, m.d.x);
        b = vec4(m.a.y, m.b.y, m.c.y, m.d.y);
        c = vec4(m.a.z, m.b.z, m.c.z, m.d.z);
        d = vec4(m.a.w, m.b.w, m.c.w, m.d.w);
    }

    void frustum(float left, float right, float bottom, float top, float znear, float zfar)
    {
        float width = right - left, height = top - bottom, zrange = znear - zfar;
        a = vec4(2*znear/width, 0, 0, 0);
        b = vec4(0, 2*znear/height, 0, 0);
        c = vec4((right + left)/width, (top + bottom)/height, (zfar + znear)/zrange, -1);
        d = vec4(0, 0, 2*znear*zfar/zrange, 0);
    }

    void perspective(float fovy, float aspect, float znear, float zfar);

    void ortho(float left, float right, float bottom, float top, float znear, float zfar)
    {
        float width = right - left, height = top - bottom, zrange = znear - zfar;
        a = vec4(2/width, 0, 0, 0);
        b = vec4(0, 2/height, 0, 0);
        c = vec4(0, 0, 2/zrange, 0);
        d = vec4(-(right+left)/width, -(top+bottom)/height, (zfar+znear)/zrange, 1);
    }

    void clip(const plane &p, const matrix4 &m);

    void transform(const vec &in, vec &out) const
    {
        out = vec(a).mul(in.x).add(vec(b).mul(in.y)).add(vec(c).mul(in.z)).add(vec(d));
    }

    void transform(const vec4 &in, vec &out) const
    {
        out = vec(a).mul(in.x).add(vec(b).mul(in.y)).add(vec(c).mul(in.z)).add(vec(d).mul(in.w));
    }

    void transform(const vec &in, vec4 &out) const
    {
        out = vec4(a).mul(in.x).madd(b, in.y).madd(c, in.z).add(d);
    }

    void transform(const vec4 &in, vec4 &out) const
    {
        out = vec4(a).mul(in.x).madd(b, in.y).madd(c, in.z).madd(d, in.w);
    }

    template<class T, class U> T transform(const U &in) const
    {
        T v;
        transform(in, v);
        return v;
    }

    template<class T> vec perspectivetransform(const T &in) const
    {
        vec4 v;
        transform(in, v);
        return vec(v).div(v.w);
    }

    void transformnormal(const vec &in, vec &out) const
    {
        out = vec(a).mul(in.x).add(vec(b).mul(in.y)).add(vec(c).mul(in.z));
    }

    void transformnormal(const vec &in, vec4 &out) const
    {
        out = vec4(a).mul(in.x).madd(b, in.y).madd(c, in.z);
    }

    template<class T, class U> T transformnormal(const U &in) const
    {
        T v;
        transformnormal(in, v);
        return v;
    }

    void transposedtransform(const vec &in, vec &out) const
    {
        vec p = vec(in).sub(vec(d));
        out.x = a.dot3(p);
        out.y = b.dot3(p);
        out.z = c.dot3(p);
    }

    void transposedtransformnormal(const vec &in, vec &out) const
    {
        out.x = a.dot3(in);
        out.y = b.dot3(in);
        out.z = c.dot3(in);
    }

    void transposedtransform(const plane &in, plane &out) const;

    float getscale() const;

    vec gettranslation() const
    {
        return vec(d);
    }

    vec4 rowx() const { return vec4(a.x, b.x, c.x, d.x); }
    vec4 rowy() const { return vec4(a.y, b.y, c.y, d.y); }
    vec4 rowz() const { return vec4(a.z, b.z, c.z, d.z); }
    vec4 roww() const { return vec4(a.w, b.w, c.w, d.w); }

    bool invert(const matrix4 &m, double mindet = 1.0e-12);

    vec2 lineardepthscale() const;
};
