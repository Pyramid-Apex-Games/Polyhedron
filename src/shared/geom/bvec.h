#pragma once
#include "shared/types.h"

struct bvec4;
struct bvec;
struct vec;


struct bvec
{
    union
    {
        struct { uchar x, y, z; };
        struct { uchar r, g, b; };
        uchar v[3];
    };

    bvec() {}
    bvec(uchar x, uchar y, uchar z) : x(x), y(y), z(z) {}
    explicit bvec(const vec &v);
    explicit bvec(const bvec4 &v);

    uchar &operator[](int i)       { return v[i]; }
    uchar  operator[](int i) const { return v[i]; }

    bool operator==(const bvec &v) const { return x==v.x && y==v.y && z==v.z; }
    bool operator!=(const bvec &v) const { return x!=v.x || y!=v.y || z!=v.z; }

    bool iszero() const { return x==0 && y==0 && z==0; }

    vec tonormal() const;

    bvec &normalize();

    void lerp(const bvec &a, const bvec &b, float t)
    {
        x = uchar(a.x + (b.x-a.x)*t);
        y = uchar(a.y + (b.y-a.y)*t);
        z = uchar(a.z + (b.z-a.z)*t);
    }

    void lerp(const bvec &a, const bvec &b, int ka, int kb, int d)
    {
        x = uchar((a.x*ka + b.x*kb)/d);
        y = uchar((a.y*ka + b.y*kb)/d);
        z = uchar((a.z*ka + b.z*kb)/d);
    }

    void flip() { x ^= 0x80; y ^= 0x80; z ^= 0x80; }

    void scale(int k, int d) { x = uchar((x*k)/d); y = uchar((y*k)/d); z = uchar((z*k)/d); }

    bvec &shl(int n) { x<<= n; y<<= n; z<<= n; return *this; }
    bvec &shr(int n) { x>>= n; y>>= n; z>>= n; return *this; }

    static bvec fromcolor(const vec &v);
    vec tocolor() const;

    static bvec from565(ushort c) { return bvec((((c>>11)&0x1F)*527 + 15) >> 6, (((c>>5)&0x3F)*259 + 35) >> 6, ((c&0x1F)*527 + 15) >> 6); }

    static bvec hexcolor(int color)
    {
        return bvec((color>>16)&0xFF, (color>>8)&0xFF, color&0xFF);
    }
    int tohexcolor() const { return (int(r)<<16)|(int(g)<<8)|int(b); }
};

