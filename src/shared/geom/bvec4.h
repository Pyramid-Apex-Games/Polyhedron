#pragma once
#include "shared/types.h"

struct bvec4;
struct bvec;
struct vec;

struct bvec4
{
    union
    {
        struct { uchar x, y, z, w; };
        struct { uchar r, g, b, a; };
        uchar v[4];
        uint mask;
    };

    bvec4() {}
    bvec4(uchar x, uchar y, uchar z, uchar w = 0) : x(x), y(y), z(z), w(w) {}
    bvec4(const bvec &v, uchar w = 0);

    uchar &operator[](int i)       { return v[i]; }
    uchar  operator[](int i) const { return v[i]; }

    bool operator==(const bvec4 &v) const { return mask==v.mask; }
    bool operator!=(const bvec4 &v) const { return mask!=v.mask; }

    bool iszero() const { return mask==0; }

    vec tonormal() const;

    void lerp(const bvec4 &a, const bvec4 &b, float t)
    {
        x = uchar(a.x + (b.x-a.x)*t);
        y = uchar(a.y + (b.y-a.y)*t);
        z = uchar(a.z + (b.z-a.z)*t);
        w = a.w;
    }

    void lerp(const bvec4 &a, const bvec4 &b, int ka, int kb, int d)
    {
        x = uchar((a.x*ka + b.x*kb)/d);
        y = uchar((a.y*ka + b.y*kb)/d);
        z = uchar((a.z*ka + b.z*kb)/d);
        w = a.w;
    }

    void lerp(const bvec4 &a, const bvec4 &b, const bvec4 &c, float ta, float tb, float tc)
    {
        x = uchar(a.x*ta + b.x*tb + c.x*tc);
        y = uchar(a.y*ta + b.y*tb + c.y*tc);
        z = uchar(a.z*ta + b.z*tb + c.z*tc);
        w = uchar(a.w*ta + b.w*tb + c.w*tc);
    }

    void flip() { mask ^= 0x80808080; }
};

