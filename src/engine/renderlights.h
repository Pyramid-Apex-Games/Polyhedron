#pragma once

extern int lighttilealignw, lighttilealignh, lighttilevieww, lighttileviewh, lighttilew, lighttileh;

extern int vieww, viewh;
template<class T>
void calctilebounds(float sx1, float sy1, float sx2, float sy2, T &bx1, T &by1, T &bx2, T &by2);

void masktiles(uint *tiles, float sx1, float sy1, float sx2, float sy2);

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

bool sphereinsidespot(const vec &dir, int spot, const vec &center, float radius);
bool bbinsidespot(const vec &origin, const vec &dir, int spot, const ivec &bbmin, const ivec &bbmax);

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

//dynlight

void updatedynlights();
int finddynlights();
bool getdynlight(int n, vec &o, float &radius, vec &color, vec &dir, int &spot, int &flags);