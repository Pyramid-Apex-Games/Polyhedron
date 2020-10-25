#pragma once
#include <algorithm>
#include <cstring>
#include <cstdarg>
#include <cstdio>

#ifdef __GNUC__
#define PRINTFARGS(fmt, args) __attribute__((format(printf, fmt, args)))
#else
#define PRINTFARGS(fmt, args)
#endif

#define MAXSTRLEN 260
typedef char cubestr[MAXSTRLEN];

inline void vformatcubestr(char *d, const char *fmt, va_list v, int len) { vsnprintf(d, len, fmt, v); d[len-1] = 0; }
template<size_t N> inline void vformatcubestr(char (&d)[N], const char *fmt, va_list v) { vformatcubestr(d, fmt, v, N); }

inline char *copycubestr(char *d, const char *s, size_t len)
{
    size_t slen = std::min(strlen(s), len-1);
    memcpy(d, s, slen);
    d[slen] = 0;
    return d;
}
template<size_t N> inline char *copycubestr(char (&d)[N], const char *s) { return copycubestr(d, s, N); }

inline char *concatcubestr(char *d, const char *s, size_t len)
{
    size_t used = strlen(d);
    if (used < len)
    {
        return copycubestr(d+used, s, len-used);
    }
    else
    {
        return d;
    }
}

template<size_t N> inline char *concatcubestr(char (&d)[N], const char *s) { return concatcubestr(d, s, N); }

inline char *prependcubestr(char *d, const char *s, size_t len)
{
    size_t slen = std::min(strlen(s), len);
    memmove(&d[slen], d, std::min(len - slen, strlen(d) + 1));
    memcpy(d, s, slen);
    d[len-1] = 0;
    return d;
}
template<size_t N> inline char *prependcubestr(char (&d)[N], const char *s) { return prependcubestr(d, s, N); }

inline void nformatcubestr(char *d, int len, const char *fmt, ...) PRINTFARGS(3, 4);
inline void nformatcubestr(char *d, int len, const char *fmt, ...)
{
    va_list v;
    va_start(v, fmt);
    vformatcubestr(d, fmt, v, len);
    va_end(v);
}

template<size_t N> inline void formatcubestr(char (&d)[N], const char *fmt, ...) PRINTFARGS(2, 3);
template<size_t N> inline void formatcubestr(char (&d)[N], const char *fmt, ...)
{
    va_list v;
    va_start(v, fmt);
    vformatcubestr(d, fmt, v, int(N));
    va_end(v);
}

template<size_t N> inline void concformatcubestr(char (&d)[N], const char *fmt, ...) PRINTFARGS(2, 3);
template<size_t N> inline void concformatcubestr(char (&d)[N], const char *fmt, ...)
{
    va_list v;
    va_start(v, fmt);
    int len = strlen(d);
    vformatcubestr(d + len, fmt, v, int(N) - len);
    va_end(v);
}

extern char *tempformatcubestr(const char *fmt, ...) PRINTFARGS(1, 2);

#define defformatcubestr(d,...) cubestr d; formatcubestr(d, __VA_ARGS__)
#define defvformatcubestr(d,last,fmt) cubestr d; { va_list ap; va_start(ap, last); vformatcubestr(d, fmt, ap); va_end(ap); }

template<size_t N> inline bool matchcubestr(const char *s, size_t len, const char (&d)[N])
{
    return len == N-1 && !memcmp(s, d, N-1);
}

inline char *newcubestr(size_t l)                { return new char[l+1]; }
inline char *newcubestr(const char *s, size_t l) { return copycubestr(newcubestr(l), s, l+1); }
inline char *newcubestr(const char *s)           { size_t l = strlen(s); char *d = newcubestr(l); memcpy(d, s, l+1); return d; }

inline char *newconcatcubestr(const char *s, const char *t)
{
    size_t slen = strlen(s), tlen = strlen(t);
    char *r = newcubestr(slen + tlen);
    memcpy(r, s, slen);
    memcpy(&r[slen], t, tlen);
    r[slen+tlen] = '\0';
    return r;
}


struct cubestrslice
{
    const char *str;
    int len;
    cubestrslice() {}
    cubestrslice(const char *str, int len) : str(str), len(len) {}
    cubestrslice(const char *str, const char *end) : str(str), len(int(end-str)) {}

    const char *end() const { return &str[len]; }
};

inline char *newcubestr(const cubestrslice &s) { return newcubestr(s.str, s.len); }
inline const char *cubestrptr(const char *s) { return s; }
inline const char *cubestrptr(const cubestrslice &s) { return s.str; }
inline int cubestrlen(const char *s) { return int(strlen(s)); }
inline int cubestrlen(const cubestrslice &s) { return s.len; }

inline char *copycubestr(char *d, const cubestrslice &s, size_t len)
{
    size_t slen = std::min(size_t(s.len), len-1);
    memcpy(d, s.str, slen);
    d[slen] = 0;
    return d;
}
template<size_t N> inline char *copycubestr(char (&d)[N], const cubestrslice &s) { return copycubestr(d, s, N); }


enum
{
    CT_PRINT   = 1<<0,
    CT_SPACE   = 1<<1,
    CT_DIGIT   = 1<<2,
    CT_ALPHA   = 1<<3,
    CT_LOWER   = 1<<4,
    CT_UPPER   = 1<<5,
    CT_UNICODE = 1<<6
};

extern const uchar cubectype[256];
static inline int iscubeprint(uchar c) { return cubectype[c]&CT_PRINT; }
static inline int iscubespace(uchar c) { return cubectype[c]&CT_SPACE; }
static inline int iscubealpha(uchar c) { return cubectype[c]&CT_ALPHA; }
static inline int iscubealnum(uchar c) { return cubectype[c]&(CT_ALPHA|CT_DIGIT); }
static inline int iscubelower(uchar c) { return cubectype[c]&CT_LOWER; }
static inline int iscubeupper(uchar c) { return cubectype[c]&CT_UPPER; }
static inline int iscubepunct(uchar c) { return cubectype[c] == CT_PRINT; }
static inline int cube2uni(uchar c)
{
    extern const int cube2unichars[256];
    return cube2unichars[c];
}
static inline uchar uni2cube(int c)
{
    extern const int uni2cubeoffsets[8];
    extern const uchar uni2cubechars[];
    return uint(c) <= 0x7FF ? uni2cubechars[uni2cubeoffsets[c>>8] + (c&0xFF)] : 0;
}
static inline uchar cubelower(uchar c)
{
    extern const uchar cubelowerchars[256];
    return cubelowerchars[c];
}
static inline uchar cubeupper(uchar c)
{
    extern const uchar cubeupperchars[256];
    return cubeupperchars[c];
}

extern size_t decodeutf8(uchar *dst, size_t dstlen, const uchar *src, size_t srclen, size_t *carry = NULL);
extern size_t encodeutf8(uchar *dstbuf, size_t dstlen, const uchar *srcbuf, size_t srclen, size_t *carry = NULL);
