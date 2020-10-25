#pragma once


struct usvec
{
    union
    {
        struct { ushort x, y, z; };
        ushort v[3];
    };

    ushort &operator[](int i) { return v[i]; }
    ushort operator[](int i) const { return v[i]; }
};