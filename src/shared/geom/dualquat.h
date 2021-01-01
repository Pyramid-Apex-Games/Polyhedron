#pragma once
#include "quat.h"

struct matrix4x3;

struct dualquat
{
    quat real, dual;

    dualquat() {}
    dualquat(const quat &q, const vec &p);
    explicit dualquat(const quat &q) : real(q), dual(0, 0, 0, 0) {}
    explicit dualquat(const matrix4x3 &m);

    dualquat &mul(float k) { real.mul(k); dual.mul(k); return *this; }
    dualquat &add(const dualquat &d) { real.add(d.real); dual.add(d.dual); return *this; }

    dualquat &lerp(const dualquat &to, float t)
    {
        float k = real.dot(to.real) < 0 ? -t : t;
        real.mul(1-t).madd(to.real, k);
        dual.mul(1-t).madd(to.dual, k);
        return *this;
    }
    dualquat &lerp(const dualquat &from, const dualquat &to, float t)
    {
        float k = from.real.dot(to.real) < 0 ? -t : t;
        (real = from.real).mul(1-t).madd(to.real, k);
        (dual = from.dual).mul(1-t).madd(to.dual, k);
        return *this;
    }

    dualquat &invert()
    {
        real.invert();
        dual.invert();
        dual.msub(real, 2*real.dot(dual));
        return *this;
    }

    void mul(const dualquat &p, const dualquat &o)
    {
        real.mul(p.real, o.real);
        dual.mul(p.real, o.dual).add(quat().mul(p.dual, o.real));
    }
    void mul(const dualquat &o) { mul(dualquat(*this), o); }

    void mulorient(const quat &q)
    {
        real.mul(q, quat(real));
        dual.mul(quat(q).invert(), quat(dual));
    }

    void mulorient(const quat &q, const dualquat &base)
    {
        quat trans;
        trans.mul(base.dual, quat(base.real).invert());
        dual.mul(quat(q).invert(), quat().mul(real, trans).add(dual));

        real.mul(q, quat(real));
        dual.add(quat().mul(real, trans.invert())).msub(real, 2*base.real.dot(base.dual));
    }

    void normalize()
    {
        float invlen = 1/real.magnitude();
        real.mul(invlen);
        dual.mul(invlen);
    }

    void translate(const vec &p);

    void scale(float k)
    {
        dual.mul(k);
    }

    void fixantipodal(const dualquat &d)
    {
        if(real.dot(d.real) < 0)
        {
            real.neg();
            dual.neg();
        }
    }

    void accumulate(const dualquat &d, float k)
    {
        if(real.dot(d.real) < 0) k = -k;
        real.madd(d.real, k);
        dual.madd(d.dual, k);
    }

    vec transform(const vec &v) const;

    quat transform(const quat &q) const
    {
        return quat().mul(real, q);
    }

    vec transposedtransform(const vec &v) const;

    vec transformnormal(const vec &v) const;

    vec transposedtransformnormal(const vec &v) const;

    vec gettranslation() const;
};
