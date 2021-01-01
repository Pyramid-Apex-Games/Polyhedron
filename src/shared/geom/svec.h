#pragma once

struct ivec;

struct svec
{
    union
    {
        struct { short x, y, z; };
        short v[3];
    };

    svec() {}
    svec(short x, short y, short z) : x(x), y(y), z(z) {}
    explicit svec(const ivec &v);

    short &operator[](int i) { return v[i]; }
    short operator[](int i) const { return v[i]; }
};