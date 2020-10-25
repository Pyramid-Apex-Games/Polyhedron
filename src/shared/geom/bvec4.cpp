#include "bvec4.h"
#include "bvec.h"
#include "vec.h"

bvec4::bvec4(const bvec &v, uchar w) : x(v.x), y(v.y), z(v.z), w(w) {}
vec bvec4::tonormal() const { return vec(x*(2.0f/255.0f)-1.0f, y*(2.0f/255.0f)-1.0f, z*(2.0f/255.0f)-1.0f); }
