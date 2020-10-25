#pragma once
#include "shared/types.h"

struct half
{
    ushort val;

    half() {}
    half(float f);

    bool operator==(const half &h) const { return val == h.val; }
    bool operator!=(const half &h) const { return val != h.val; }
};
