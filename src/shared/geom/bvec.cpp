#include "bvec.h"
#include "bvec4.h"
#include "vec.h"

bvec::bvec(const bvec4 &v) : x(v.x), y(v.y), z(v.z) {}
bvec::bvec(const vec &v) : x(uchar((v.x+1)*(255.0f/2.0f))), y(uchar((v.y+1)*(255.0f/2.0f))), z(uchar((v.z+1)*(255.0f/2.0f))) {}

vec bvec::tonormal() const { return vec(x*(2.0f/255.0f)-1.0f, y*(2.0f/255.0f)-1.0f, z*(2.0f/255.0f)-1.0f); }

bvec &bvec::normalize()
{
    vec n(x-127.5f, y-127.5f, z-127.5f);
    float mag = 127.5f/n.magnitude();
    x = uchar(n.x*mag+127.5f);
    y = uchar(n.y*mag+127.5f);
    z = uchar(n.z*mag+127.5f);
    return *this;
}

bvec bvec::fromcolor(const vec &v) { return bvec(uchar(v.x*255.0f), uchar(v.y*255.0f), uchar(v.z*255.0f)); }

vec bvec::tocolor() const { return vec(x*(1.0f/255.0f), y*(1.0f/255.0f), z*(1.0f/255.0f)); }