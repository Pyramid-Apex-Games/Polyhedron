#pragma once

struct vec4;

struct dvec4
{
    double x, y, z, w;

    dvec4() {}
    dvec4(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}
    dvec4(const vec4 &v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

    template<class B> dvec4 &madd(const dvec4 &a, const B &b) { return add(dvec4(a).mul(b)); }
    dvec4 &mul(double f)       { x *= f; y *= f; z *= f; w *= f; return *this; }
    dvec4 &mul(const dvec4 &o) { x *= o.x; y *= o.y; z *= o.z; w *= o.w; return *this; }
    dvec4 &add(double f)       { x += f; y += f; z += f; w += f; return *this; }
    dvec4 &add(const dvec4 &o) { x += o.x; y += o.y; z += o.z; w += o.w; return *this; }

    operator vec4() const { return vec4(x, y, z, w); }
};
