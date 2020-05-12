#pragma once

#define LIGHTTILE_MAXW 16
#define LIGHTTILE_MAXH 16

extern int lighttilealignw, lighttilealignh, lighttilevieww, lighttileviewh, lighttilew, lighttileh;

template<class T>
static inline void calctilebounds(float sx1, float sy1, float sx2, float sy2, T &bx1, T &by1, T &bx2, T &by2)
{
    int tx1 = max(int(floor(((sx1 + 1)*0.5f*vieww)/lighttilealignw)), 0),
            ty1 = max(int(floor(((sy1 + 1)*0.5f*viewh)/lighttilealignh)), 0),
            tx2 = min(int(ceil(((sx2 + 1)*0.5f*vieww)/lighttilealignw)), lighttilevieww),
            ty2 = min(int(ceil(((sy2 + 1)*0.5f*viewh)/lighttilealignh)), lighttileviewh);
    bx1 = T((tx1 * lighttilew) / lighttilevieww);
    by1 = T((ty1 * lighttileh) / lighttileviewh);
    bx2 = T((tx2 * lighttilew + lighttilevieww - 1) / lighttilevieww);
    by2 = T((ty2 * lighttileh + lighttileviewh - 1) / lighttileviewh);
}

static inline void masktiles(uint *tiles, float sx1, float sy1, float sx2, float sy2)
{
    int tx1, ty1, tx2, ty2;
    calctilebounds(sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2);
    for(int ty = ty1; ty < ty2; ty++) tiles[ty] |= ((1<<(tx2-tx1))-1)<<tx1;
}

enum { SM_NONE = 0, SM_REFLECT, SM_CUBEMAP, SM_CASCADE, SM_SPOT };

extern int shadowmapping;

extern vec shadoworigin, shadowdir;
extern float shadowradius, shadowbias;
extern int shadowside, shadowspot;
extern matrix4 shadowmatrix;

void loaddeferredlightshaders();
void cleardeferredlightshaders();
void clearshadowcache();

void rendervolumetric();
void cleanupvolumetric();

void findshadowvas();
void findshadowmms();

int calcshadowinfo(const entities::classes::CoreEntity *e, vec &origin, float &radius, vec &spotloc, int &spotangle, float &bias);
int dynamicshadowvabounds(int mask, vec &bbmin, vec &bbmax);
void rendershadowmapworld();
void batchshadowmapmodels(bool skipmesh = false);
void rendershadowatlas();
void renderrsmgeom(bool dyntex = false);
bool useradiancehints();
void renderradiancehints();
void clearradiancehintscache();
void cleanuplights();
void workinoq();

int calcbbsidemask(const vec &bbmin, const vec &bbmax, const vec &lightpos, float lightradius, float bias);
int calcspheresidemask(const vec &p, float radius, float bias);
int calctrisidemask(const vec &p1, const vec &p2, const vec &p3, float bias);
int cullfrustumsides(const vec &lightpos, float lightradius, float size, float border);
int calcbbcsmsplits(const ivec &bbmin, const ivec &bbmax);
int calcspherecsmsplits(const vec &center, float radius);
int calcbbrsmsplits(const ivec &bbmin, const ivec &bbmax);
int calcspherersmsplits(const vec &center, float radius);

static inline bool sphereinsidespot(const vec &dir, int spot, const vec &center, float radius)
{
    const vec2 &sc = sincos360[spot];
    float cdist = dir.dot(center), cradius = radius + sc.y*cdist;
    return sc.x*sc.x*(center.dot(center) - cdist*cdist) <= cradius*cradius;
}
static inline bool bbinsidespot(const vec &origin, const vec &dir, int spot, const ivec &bbmin, const ivec &bbmax)
{
    vec radius = vec(ivec(bbmax).sub(bbmin)).mul(0.5f), center = vec(bbmin).add(radius);
    return sphereinsidespot(dir, spot, center.sub(origin), radius.magnitude());
}

extern matrix4 worldmatrix, screenmatrix;

extern int transparentlayer;

extern int gw, gh, gdepthformat, ghasstencil;
extern GLuint gdepthtex, gcolortex, gnormaltex, gglowtex, gdepthrb, gstencilrb;
extern int msaasamples, msaalight;
extern GLuint msdepthtex, mscolortex, msnormaltex, msglowtex, msdepthrb, msstencilrb;
extern vector<vec2> msaapositions;
enum { AA_UNUSED = 0, AA_LUMA, AA_MASKED, AA_SPLIT, AA_SPLIT_LUMA, AA_SPLIT_MASKED };

void cleanupgbuffer();
void initgbuffer();
bool usepacknorm();
void maskgbuffer(const char *mask);
void bindgdepth();
void preparegbuffer(bool depthclear = true);
void rendergbuffer(bool depthclear = true);
void shadesky();
void shadegbuffer();
void shademinimap(const vec &color = vec(-1, -1, -1));
void shademodelpreview(int x, int y, int w, int h, bool background = true, bool scissor = false);
void rendertransparent();
void renderao();
void loadhdrshaders(int aa = AA_UNUSED);
void processhdr(GLuint outfbo = 0, int aa = AA_UNUSED);
void copyhdr(int sw, int sh, GLuint fbo, int dw = 0, int dh = 0, bool flipx = false, bool flipy = false, bool swapxy = false);
void setuplights();
void setupgbuffer();
GLuint shouldscale();
void doscale(GLuint outfbo = 0);
bool debuglights();
void cleanuplights();

extern int avatarmask;

bool useavatarmask();
void enableavatarmask();
void disableavatarmask();