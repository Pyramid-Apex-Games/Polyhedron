// generic useful stuff for any C++ program

#ifndef _TOOLS_H
#define _TOOLS_H
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <utility>
#include <algorithm>
#include <enet/enet.h>

#include "engine/includegl.h"
#include "zlib.h"
#include "types.h"


#if 0
void *operator new(size_t, bool);
void *operator new[](size_t, bool);
inline void *operator new(size_t, void *p) { return p; }
inline void *operator new[](size_t, void *p) { return p; }
inline void operator delete(void *, void *) {}
inline void operator delete[](void *, void *) {}
#endif

using std::min;
using std::max;
using std::swap;

template<class T, class U>
static inline T clamp(T a, U b, U c)
{
    return max(T(b), min(a, T(c)));
}

#define rnd(x) ((int)(randomMT()&0x7FFFFFFF)%(x))
#define rndscale(x) (float((randomMT()&0x7FFFFFFF)*double(x)/double(0x7FFFFFFF)))
#define detrnd(s, x) ((int)(((((uint)(s))*1103515245+12345)>>16)%(x)))

#include "loops.h"

#include "geom/constants.h"
#include "shared/tools/databuf.h"


#include "tools/macros.h"
#include "tools/databuf.h"
#include "tools/packetbuf.h"
#include "tools/cubestr.h"
#include "tools/sort.h"
#include "tools/loop.h"
#include "tools/vector.h"
#include "tools/hash.h"
#include "tools/queue.h"
#include "tools/endianness.h"
#include "tools/isclass.h"

/* workaround for some C platforms that have these two functions as macros - not used anywhere */
#ifdef getchar
#undef getchar
#endif
#ifdef putchar
#undef putchar
#endif
#include "tools/stream.h"
#include "tools/streambuf.h"

extern void seedMT(uint seed);
extern uint randomMT();

extern void putint(ucharbuf &p, int n);
extern void putint(packetbuf &p, int n);
extern void putint(vector<uchar> &p, int n);
extern int getint(ucharbuf &p);
extern void putuint(ucharbuf &p, int n);
extern void putuint(packetbuf &p, int n);
extern void putuint(vector<uchar> &p, int n);
extern int getuint(ucharbuf &p);
extern void putfloat(ucharbuf &p, float f);
extern void putfloat(packetbuf &p, float f);
extern void putfloat(vector<uchar> &p, float f);
extern float getfloat(ucharbuf &p);
extern void sendcubestr(const char *t, ucharbuf &p);
extern void sendcubestr(const char *t, packetbuf &p);
extern void sendcubestr(const char *t, vector<uchar> &p);
extern void getcubestr(char *t, ucharbuf &p, size_t len);
template<size_t N> static inline void getcubestr(char (&t)[N], ucharbuf &p) { getcubestr(t, p, N); }
extern void filtertext(char *dst, const char *src, bool whitespace, bool forcespace, size_t len);
template<size_t N> static inline void filtertext(char (&dst)[N], const char *src, bool whitespace = true, bool forcespace = false) { filtertext(dst, src, whitespace, forcespace, N-1); }

struct ipmask
{
    enet_uint32 ip, mask;

    void parse(const char *name);
    int print(char *buf) const;
    bool check(enet_uint32 host) const { return (host & mask) == ip; }
};

#endif

