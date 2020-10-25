#pragma once
#include "shared/geom/matrix4.h"
#include "shared/geom/vec.h"
#include "shared/geom/bvec.h"
#include "shared/entities/DynamicEntity.h"
#include "glad/glad.h"

extern int glversion, glslversion, glcompat;
extern int maxdrawbufs, maxdualdrawbufs;

enum { DRAWTEX_NONE = 0, DRAWTEX_ENVMAP, DRAWTEX_MINIMAP, DRAWTEX_MODELPREVIEW };

extern int vieww, viewh;
extern int screenw, screenh, renderw, renderh, hudw, hudh;
extern int fov;
extern float curfov, fovy, aspect, forceaspect;
extern float nearplane;
extern int farplane;
extern bool hdrfloat;
extern float ldrscale, ldrscaleb;
extern int drawtex;
extern const matrix4 viewmatrix, invviewmatrix;
extern matrix4 cammatrix, projmatrix, camprojmatrix, invcammatrix, invcamprojmatrix, invprojmatrix;
extern int fog;
extern bvec fogcolour;
extern vec curfogcolor;
extern int wireframe;

extern int xtraverts, xtravertsva;


extern int glerr;
void glerror(const char *file, int line, GLenum error);

#define GLERROR do { if(glerr) { GLenum error = glGetError(); if(error != GL_NO_ERROR) glerror(__FILE__, __LINE__, error); } } while(0)

void gl_checkextensions();
void gl_init();
void gl_resize();
void gl_drawview();
void gl_drawmainmenu();
void gl_drawhud();
void gl_setupframe(bool force = false);
void gl_drawframe();
void cleanupgl();
void drawminimap();
void enablepolygonoffset(GLenum type);
void disablepolygonoffset(GLenum type);
bool calcspherescissor(const vec &center, float size, float &sx1, float &sy1, float &sx2, float &sy2, float &sz1, float &sz2);
bool calcbbscissor(const ivec &bbmin, const ivec &bbmax, float &sx1, float &sy1, float &sx2, float &sy2);
bool calcspotscissor(const vec &origin, float radius, const vec &dir, int spot, const vec &spotx, const vec &spoty, float &sx1, float &sy1, float &sx2, float &sy2, float &sz1, float &sz2);
void screenquad();
void screenquad(float sw, float sh);
void screenquadflipped(float sw, float sh);
void screenquad(float sw, float sh, float sw2, float sh2);
void screenquadoffset(float x, float y, float w, float h);
void screenquadoffset(float x, float y, float w, float h, float x2, float y2, float w2, float h2);
void hudquad(float x, float y, float w, float h, float tx = 0, float ty = 0, float tw = 1, float th = 1);
void debugquad(float x, float y, float w, float h, float tx = 0, float ty = 0, float tw = 1, float th = 1);
void recomputecamera();
float calcfrustumboundsphere(float nearplane, float farplane,  const vec &pos, const vec &view, vec &center);
void setfogcolor(const vec &v);
void zerofogcolor();
void resetfogcolor();
float calcfogdensity(float dist);
float calcfogcull();
void writecrosshairs(stream *f);
void renderavatar();

namespace modelpreview
{
    void start(int x, int y, int w, int h, bool background = true, bool scissor = false);
    void end();
}

struct timer;
timer *begintimer(const char *name, bool gpu = true);
void endtimer(timer *t);