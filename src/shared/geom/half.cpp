#include "half.h"
#include <algorithm>

half::half(float f)
{
    union { int i; float f; } conv;
    conv.f = f;
    ushort signbit = (conv.i>>(31-15)) & (1<<15), mantissa = (conv.i>>(23-10)) & 0x3FF;
    int exponent = ((conv.i>>23)&0xFF) - 127 + 15;
    if(exponent <= 0)
    {
        mantissa |= 0x400;
        mantissa >>= std::min(1-exponent, 10+1);
        exponent = 0;
    }
    else if(exponent >= 0x1F)
    {
        mantissa = 0;
        exponent = 0x1F;
    }
    val = signbit | (ushort(exponent)<<10) | mantissa;
}