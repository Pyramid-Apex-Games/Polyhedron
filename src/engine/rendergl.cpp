// rendergl.cpp: core opengl rendering stuff

#include "shared/cube.h"
#include "shared/ents.h"
#include "engine/texture.h"
#include "engine/pvs.h"
#include "engine/engine/font.h"
#include "engine/rendergl.h"
#include "engine/renderlights.h"
#include "engine/aa.h"
#include "engine/octaedit.h"
#include "engine/octarender.h"
#include "engine/renderva.h"
#include "engine/material.h"
#include "engine/water.h"
#include "engine/console.h"
#include "engine/rendermodel.h"
#include "engine/renderparticles.h"
#include "engine/rendersky.h"
#include "engine/ui.h"
#include "engine/menus.h"
#include "engine/grass.h"
#include "engine/blend.h"
#include "engine/main/Application.h"
#include "engine/main/Input.h"
#include "engine/main/Window.h"
#include "engine/main/GLContext.h"
#include "engine/main/Renderer.h"
#include "engine/main/FPS.h"
#include "engine/main/Clock.h"
#include "engine/main/Compatibility.h"
#include "engine/hud.h"
#include "engine/GLFeatures.h"
#include "engine/Camera.h"
#include "game/game.h"
#include "game/entities/SkeletalEntity.h"
#include "game/game.h"
#include "game/entities/SkeletalEntity.h"

int hasstencil = 0;


void *getprocaddress(const char *name)
{
    return SDL_GL_GetProcAddress(name);
}

VAR(glerr, 0, 0, 1);

void glerror(const char *file, int line, GLenum error)
{
    const char *desc = "unknown";
    switch(error)
    {
    case GL_NO_ERROR: desc = "no error"; break;
    case GL_INVALID_ENUM: desc = "invalid enum"; break;
    case GL_INVALID_VALUE: desc = "invalid value"; break;
    case GL_INVALID_OPERATION: desc = "invalid operation"; break;
    case GL_STACK_OVERFLOW: desc = "stack overflow"; break;
    case GL_STACK_UNDERFLOW: desc = "stack underflow"; break;
    case GL_OUT_OF_MEMORY: desc = "out of memory"; break;
    }
    printf("GL error: %s:%d: %s (%x)\n", file, line, desc, error);
}


VAR(dbgexts, 0, 1, 1);

SCRIPTEXPORT void glext(char *ext)
{
    intret(has_ext(ext) ? 1 : 0);
}

struct timer
{
    enum { MAXQUERY = 4 };

    const char *name;
    bool gpu;
    GLuint query[MAXQUERY];
    int waiting;
    uint starttime;
    float result, print;
};
static vector<timer> timers;
static vector<int> timerorder;
static int timercycle = 0;

extern int usetimers;

timer *findtimer(const char *name, bool gpu)
{
    loopv(timers) if(!strcmp(timers[i].name, name) && timers[i].gpu == gpu)
    {
        timerorder.removeobj(i);
        timerorder.emplace_back(i);
        return &timers[i];
    }
    timerorder.emplace_back(timers.size());
    timer &t = timers.emplace_back();
    t.name = name;
    t.gpu = gpu;
    memset(t.query, 0, sizeof(t.query));
    if(gpu){
        glCheckError(glGenQueries_(timer::MAXQUERY, t.query));
    }
    t.waiting = 0;
    t.starttime = 0;
    t.result = -1;
    t.print = -1;
    return &t;
}

timer *begintimer(const char *name, bool gpu)
{
    if(!usetimers || inbetweenframes || (gpu && (!GLFeatures::HasTQ() || deferquery))) return NULL;
    timer *t = findtimer(name, gpu);
    if(t->gpu)
    {
        deferquery++;
        glCheckError(glBeginQuery_(GL_TIME_ELAPSED, t->query[timercycle]));
        t->waiting |= 1<<timercycle;
    }
    else t->starttime = getclockmillis();
    return t;
}

void endtimer(timer *t)
{
    if(!t) return;
    if(t->gpu)
    {
        glCheckError(glEndQuery_(GL_TIME_ELAPSED));
        deferquery--;
    }
    else t->result = max(float(getclockmillis() - t->starttime), 0.0f);
}

void synctimers()
{
    timercycle = (timercycle + 1) % timer::MAXQUERY;

    loopv(timers)
    {
        timer &t = timers[i];
        if(t.waiting&(1<<timercycle))
        {
            GLint available = 0;
#ifndef OPEN_GL_ES
            while(!available) {
                glCheckError(glGetQueryObjectiv_(t.query[timercycle], GL_QUERY_RESULT_AVAILABLE, &available));
            }
#endif
            GLuint64EXT result = 0;
#ifndef OPEN_GL_ES
            glCheckError(glGetQueryObjectui64v_(t.query[timercycle], GL_QUERY_RESULT, &result));
#endif
            t.result = max(float(result) * 1e-6f, 0.0f);
            t.waiting &= ~(1<<timercycle);
        }
        else t.result = -1;
    }
}

void cleanuptimers()
{
    loopv(timers)
    {
        timer &t = timers[i];
        if(t.gpu){
            glCheckError(glDeleteQueries_(timer::MAXQUERY, t.query));
        }
    }
    timers.clear();
    timerorder.clear();
}

VARFN(timer, usetimers, 0, 0, 1, cleanuptimers());
VAR(frametimer, 0, 0, 1);
int framemillis = 0; // frame time (ie does not take into account the swap)

void printtimers(int conw, int conh)
{
    if(!frametimer && !usetimers) return;

    static int lastprint = 0;
    int offset = 0;
    if(frametimer)
    {
        static int printmillis = 0;
        if(totalmillis - lastprint >= 200) printmillis = framemillis;
        draw_textf("frame time %i ms", conw-20*FONTH, conh-FONTH*3/2-offset*9*FONTH/8, printmillis);
        offset++;
    }
    if(usetimers) loopv(timerorder)
    {
        timer &t = timers[timerorder[i]];
        if(t.print < 0 ? t.result >= 0 : totalmillis - lastprint >= 200) t.print = t.result;
        if(t.print < 0 || (t.gpu && !(t.waiting&(1<<timercycle)))) continue;
        draw_textf("%s%s %5.2f ms", conw-20*FONTH, conh-FONTH*3/2-offset*9*FONTH/8, t.name, t.gpu ? "" : " (cpu)", t.print);
        offset++;
    }
    if(totalmillis - lastprint >= 200) lastprint = totalmillis;
}

void gl_resize()
{
    gl_setupframe();
    glCheckError(glViewport(0, 0, hudw, hudh));
}

void gl_init()
{
    GLERROR;

    glCheckError(glClearColor(0, 0, 0, 0));
#ifdef OPEN_GL_ES
	glCheckError(glClearDepthf(1));
#else
    glCheckError(glClearDepth(1));
#endif
    glCheckError(glClearStencil(0));
    glCheckError(glDepthFunc(GL_LESS));
    glCheckError(glDisable(GL_DEPTH_TEST));
    glCheckError(glDisable(GL_STENCIL_TEST));
    glCheckError(glStencilFunc(GL_ALWAYS, 0, ~0));
    glCheckError(glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP));

#ifndef OPEN_GL_ES
    glCheckError(glEnable(GL_LINE_SMOOTH));
#endif
    //glCheckError(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));

    glCheckError(glFrontFace(GL_CW));
    glCheckError(glCullFace(GL_BACK));
    glCheckError(glDisable(GL_CULL_FACE));

    gle::setup();
    setupshaders();
    setuptexcompress();

    GLERROR;

    gl_resize();
}

VAR(wireframe, 0, 0, 1);

SCRIPTEXPORT void getcamyaw()
{
    assert(Camera::GetActiveCamera());

    floatret(Camera::GetActiveCamera()->d.x);
}

SCRIPTEXPORT void getcampitch()
{
    assert(Camera::GetActiveCamera());

    floatret(Camera::GetActiveCamera()->d.y);
}

SCRIPTEXPORT void getcamroll()
{
    assert(Camera::GetActiveCamera());

    floatret(Camera::GetActiveCamera()->d.z);
}

SCRIPTEXPORT void getcampos()
{
    const auto& activeCamera = Camera::GetActiveCamera();
    assert(activeCamera);
    defformatcubestr(pos, "%s %s %s", floatstr(activeCamera->o.x), floatstr(activeCamera->o.y), floatstr(activeCamera->o.z));
    result(pos);
}

//vec worldpos, camdir, camright, camup;

void setcammatrix()
{
    Camera::UpdateAll();
}

void setcamprojmatrix(bool init = true, bool flush = false)
{
    if(init)
    {
        setcammatrix();
    }

    jitteraa();

    auto activeCamera = Camera::GetActiveCamera();
    if (!activeCamera)
        return;

    auto& cammatrix = activeCamera->GetMatrix();
    camprojmatrix.muld(projmatrix, cammatrix);

    if(init)
    {
        invcammatrix.invert(cammatrix);
        invprojmatrix.invert(projmatrix);
        invcamprojmatrix.invert(camprojmatrix);
    }

    GLOBALPARAM(camprojmatrix, camprojmatrix);
    GLOBALPARAM(lineardepthscale, projmatrix.lineardepthscale()); //(invprojmatrix.c.z, invprojmatrix.d.z));

    if(flush && Shader::lastshader) Shader::lastshader->flushparams();
}

int vieww = -1, viewh = -1;
float curfov, curavatarfov, fovy, aspect;
int farplane;
VARP(zoominvel, 0, 40, 500);
VARP(zoomoutvel, 0, 50, 500);
VARP(zoomfov, 10, 42, 90);
VARP(fov, 10, 100, 150);
VAR(avatarzoomfov, 1, 1, 1);
VAR(avatarfov, 10, 40, 100);
FVAR(avatardepth, 0, 0.7f, 1);
FVARNP(aspect, forceaspect, 0, 0, 1e3f);

static float zoomprogress = 0;
VAR(zoom, -1, 0, 1);

void disablezoom()
{
    zoom = 0;
    zoomprogress = 0;
}

void computezoom()
{
    if(!zoom) { zoomprogress = 0; curfov = fov; curavatarfov = avatarfov; return; }
    if(zoom > 0) zoomprogress = zoominvel ? min(zoomprogress + float(elapsedtime) / zoominvel, 1.0f) : 1;
    else
    {
        zoomprogress = zoomoutvel ? max(zoomprogress - float(elapsedtime) / zoomoutvel, 0.0f) : 0;
        if(zoomprogress <= 0) zoom = 0;
    }
    curfov = zoomfov*zoomprogress + fov*(1 - zoomprogress);
    curavatarfov = avatarzoomfov*zoomprogress + avatarfov*(1 - zoomprogress);
}

FVARP(zoomsens, 1e-4f, 4.5f, 1e4f);
FVARP(zoomaccel, 0, 0, 1000);
VARP(zoomautosens, 0, 1, 1);
FVARP(sensitivity, 1e-4f, 10, 1e4f);
FVARP(sensitivityscale, 1e-4f, 100, 1e4f);
VARP(invmouse, 0, 0, 1);
FVARP(mouseaccel, 0, 0, 1000);

/*VAR(thirdperson, 0, 0, 2);
FVAR(thirdpersondistance, 0, 30, 50);
FVAR(thirdpersonup, -25, 0, 25);
FVAR(thirdpersonside, -25, 0, 25);
*/
VAR(thirdperson, 0, 1, 2);
FVAR(thirdpersondistance, 0, 10, 25);
FVAR(thirdpersonup, -25, 0, 25);
FVAR(thirdpersonside, -25, 0, 25);

bool detachedcamera = false;
bool isthirdperson() { return player!=Camera::GetActiveCamera() || detachedcamera; }

void fixcamerarange()
{
    const float MAXPITCH = 90.0f;
    const auto& activeCamera = Camera::GetActiveCamera();
    assert(activeCamera);

    if(activeCamera->d.y>MAXPITCH) activeCamera->d.y = MAXPITCH;
    if(activeCamera->d.y<-MAXPITCH) activeCamera->d.y = -MAXPITCH;
    while(activeCamera->d.x<0.0f) activeCamera->d.x += 360.0f;
    while(activeCamera->d.x>=360.0f) activeCamera->d.x -= 360.0f;
}

void modifyorient(float yaw, float pitch)
{
    const auto& activeCamera = Camera::GetActiveCamera();
    assert(activeCamera);

    activeCamera->d.x += yaw;
    activeCamera->d.y += pitch;
    fixcamerarange();
    if(activeCamera != player && !detachedcamera)
    {
        player->d.x = activeCamera->d.x;
        player->d.y = activeCamera->d.y;
    }
}

void mousemove(int dx, int dy)
{
    if(!game::allowmouselook()) return;
    float cursens = sensitivity, curaccel = mouseaccel;
    if(zoom)
    {
        if(zoomautosens)
        {
            cursens = float(sensitivity*zoomfov)/fov;
            curaccel = float(mouseaccel*zoomfov)/fov;
        }
        else
        {
            cursens = zoomsens;
            curaccel = zoomaccel;
        }
    }
    if(curaccel && curtime && (dx || dy)) cursens += curaccel * sqrtf(dx*dx + dy*dy)/curtime;
    cursens /= sensitivityscale;
    modifyorient(dx*cursens, dy*cursens*(invmouse ? 1 : -1));
}

void recomputecamera()
{
    game::setupcamera();
    computezoom();

    const auto& activeCamera = Camera::GetActiveCamera();
    assert(activeCamera);
    assert(player);

    bool allowthirdperson = true;
    bool shoulddetach = (allowthirdperson && thirdperson > 1) || game::detachcamera();

    matrix3 orient;
    orient.identity();
    orient.rotate_around_z(activeCamera->d.x *  RAD);
    orient.rotate_around_x(activeCamera->d.y *  RAD);
    orient.rotate_around_y(activeCamera->d.z * -RAD);

    vec dir = vec(orient.b).neg(), side = vec(orient.a).neg(), up = orient.c;

    if((!allowthirdperson || !thirdperson) && !shoulddetach)
    {
        activeCamera->o = player->o;

        detachedcamera = false;

        if(thirdpersonup)
        {
            activeCamera->o.add(vec(0,0,1).mul(thirdpersonup));
            activeCamera->o.add(dir.mul(-2.0f));
        }
    }
    else
    {
        activeCamera->o = player->o;

        if(detachedcamera && shoulddetach) {
        } else {
            //*activeCamera = *player;

            detachedcamera = shoulddetach;
        }

//        prepCamera1->ent_type = ENT_CAMERA;
//        activeCamera->move = -4;
//        activeCamera->eyeheight = activeCamera->aboveeye = activeCamera->radius = activeCamera->xradius = activeCamera->yradius = 2;


        if(game::collidecamera())
        {
            movecamera(activeCamera, dir, thirdpersondistance, 1);
            movecamera(activeCamera, dir, clamp(thirdpersondistance - activeCamera->o.dist(player->o), 0.0f, 1.0f), 0.1f);
            if(thirdpersonup)
            {
                vec pos = activeCamera->o;
                float dist = fabs(thirdpersonup);
                if(thirdpersonup < 0) up.neg();
                movecamera(activeCamera, up, dist, 1);
                movecamera(activeCamera, up, clamp(dist - activeCamera->o.dist(pos), 0.0f, 1.0f), 0.1f);
            }
            if(thirdpersonside)
            {
                vec pos = activeCamera->o;
                float dist = fabs(thirdpersonside);
                if(thirdpersonside < 0) side.neg();
                movecamera(activeCamera, side, dist, 1);
                movecamera(activeCamera, side, clamp(dist - activeCamera->o.dist(pos), 0.0f, 1.0f), 0.1f);
            }
        }
        else
        {
            activeCamera->o.add(vec(dir).mul(thirdpersondistance));
            if(thirdpersonup)
            {
                activeCamera->o.add(vec(up).mul(thirdpersonup));
            }
            if(thirdpersonside)
            {
                activeCamera->o.add(vec(side).mul(thirdpersonside));
            }
        }
    }

    setviewcell(activeCamera->o);
}

float calcfrustumboundsphere(float nearplane, float farplane,  const vec &pos, const vec &view, vec &center)
{
    if(drawtex == DRAWTEX_MINIMAP)
    {
        center = minimapcenter;
        return minimapradius.magnitude();
    }

    float width = tan(fov/2.0f*RAD), height = width / aspect,
          cdist = ((nearplane + farplane)/2)*(1 + width*width + height*height);
    if(cdist <= farplane)
    {
        center = vec(view).mul(cdist).add(pos);
        return vec(width*nearplane, height*nearplane, cdist-nearplane).magnitude();
    }
    else
    {
        center = vec(view).mul(farplane).add(pos);
        return vec(width*farplane, height*farplane, 0).magnitude();
    }
}

extern const matrix4 viewmatrix(vec(-1, 0, 0), vec(0, 0, 1), vec(0, -1, 0));
extern const matrix4 invviewmatrix(vec(-1, 0, 0), vec(0, 0, -1), vec(0, 1, 0));
matrix4 projmatrix, camprojmatrix, invcammatrix, invcamprojmatrix, invprojmatrix;

FVAR(nearplane, 0.01f, 0.54f, 2.0f);

vec calcavatarpos(const vec &pos, float dist)
{
    vec eyepos;
    auto activeCamera = Camera::GetActiveCamera();
    if (!activeCamera)
        return vec(0,0,0);

    auto& cammatrix = activeCamera->GetMatrix();

    cammatrix.transform(pos, eyepos);
    GLdouble ydist = nearplane * tan(curavatarfov/2*RAD), xdist = ydist * aspect;
    vec4 scrpos;
    scrpos.x = eyepos.x*nearplane/xdist;
    scrpos.y = eyepos.y*nearplane/ydist;
    scrpos.z = (eyepos.z*(farplane + nearplane) - 2*nearplane*farplane) / (farplane - nearplane);
    scrpos.w = -eyepos.z;

    vec worldpos = invcamprojmatrix.perspectivetransform(scrpos);
    vec dir = vec(worldpos).sub(activeCamera->o).rescale(dist);
    return dir.add(activeCamera->o);
}

void renderavatar()
{
    if(isthirdperson()) return;

    matrix4 oldprojmatrix = nojittermatrix;
    projmatrix.perspective(curavatarfov, aspect, nearplane, farplane);
    projmatrix.scalez(avatardepth);
    setcamprojmatrix(false);

    enableavatarmask();
    game::renderavatar();
    disableavatarmask();

    projmatrix = oldprojmatrix;
    setcamprojmatrix(false);
}

FVAR(polygonoffsetfactor, -1e4f, -3.0f, 1e4f);
FVAR(polygonoffsetunits, -1e4f, -3.0f, 1e4f);
FVAR(depthoffset, -1e4f, 0.01f, 1e4f);

matrix4 nooffsetmatrix;

void enablepolygonoffset(GLenum type)
{
    if(!depthoffset)
    {
        glCheckError(glPolygonOffset(polygonoffsetfactor, polygonoffsetunits));
        glCheckError(glEnable(type));
        return;
    }

    projmatrix = nojittermatrix;
    nooffsetmatrix = projmatrix;
    projmatrix.d.z += depthoffset * projmatrix.c.z;
    setcamprojmatrix(false, true);
}

void disablepolygonoffset(GLenum type)
{
    if(!depthoffset)
    {
        glCheckError(glDisable(type));
        return;
    }

    projmatrix = nooffsetmatrix;
    setcamprojmatrix(false, true);
}

bool calcspherescissor(const vec &center, float size, float &sx1, float &sy1, float &sx2, float &sy2, float &sz1, float &sz2)
{
    vec e;
    auto activeCamera = Camera::GetActiveCamera();
    if (!activeCamera)
        return false;

    auto& cammatrix = activeCamera->GetMatrix();

    cammatrix.transform(center, e);
    if(e.z > 2*size) { sx1 = sy1 = sz1 = 1; sx2 = sy2 = sz2 = -1; return false; }
    if(drawtex == DRAWTEX_MINIMAP)
    {
        vec dir(size, size, size);
        if(projmatrix.a.x < 0) dir.x = -dir.x;
        if(projmatrix.b.y < 0) dir.y = -dir.y;
        if(projmatrix.c.z < 0) dir.z = -dir.z;
        sx1 = max(projmatrix.a.x*(e.x - dir.x) + projmatrix.d.x, -1.0f);
        sx2 = min(projmatrix.a.x*(e.x + dir.x) + projmatrix.d.x, 1.0f);
        sy1 = max(projmatrix.b.y*(e.y - dir.y) + projmatrix.d.y, -1.0f);
        sy2 = min(projmatrix.b.y*(e.y + dir.y) + projmatrix.d.y, 1.0f);
        sz1 = max(projmatrix.c.z*(e.z - dir.z) + projmatrix.d.z, -1.0f);
        sz2 = min(projmatrix.c.z*(e.z + dir.z) + projmatrix.d.z, 1.0f);
        return sx1 < sx2 && sy1 < sy2 && sz1 < sz2;
    }
    float zzrr = e.z*e.z - size*size,
          dx = e.x*e.x + zzrr, dy = e.y*e.y + zzrr,
          focaldist = 1.0f/tan(fovy*0.5f*RAD);
    sx1 = sy1 = -1;
    sx2 = sy2 = 1;
    #define CHECKPLANE(c, dir, focaldist, low, high) \
    do { \
        float nzc = (cz*cz + 1) / (cz dir drt) - cz, \
              pz = (d##c)/(nzc*e.c - e.z); \
        if(pz > 0) \
        { \
            float c = (focaldist)*nzc, \
                  pc = pz*nzc; \
            if(pc < e.c) low = c; \
            else if(pc > e.c) high = c; \
        } \
    } while(0)
    if(dx > 0)
    {
        float cz = e.x/e.z, drt = sqrtf(dx)/size;
        CHECKPLANE(x, -, focaldist/aspect, sx1, sx2);
        CHECKPLANE(x, +, focaldist/aspect, sx1, sx2);
    }
    if(dy > 0)
    {
        float cz = e.y/e.z, drt = sqrtf(dy)/size;
        CHECKPLANE(y, -, focaldist, sy1, sy2);
        CHECKPLANE(y, +, focaldist, sy1, sy2);
    }
    float z1 = min(e.z + size, -1e-3f - nearplane), z2 = min(e.z - size, -1e-3f - nearplane);
    sz1 = (z1*projmatrix.c.z + projmatrix.d.z) / (z1*projmatrix.c.w + projmatrix.d.w);
    sz2 = (z2*projmatrix.c.z + projmatrix.d.z) / (z2*projmatrix.c.w + projmatrix.d.w);
    return sx1 < sx2 && sy1 < sy2 && sz1 < sz2;
}

bool calcbbscissor(const ivec &bbmin, const ivec &bbmax, float &sx1, float &sy1, float &sx2, float &sy2)
{
#define ADDXYSCISSOR(p) do { \
        if(p.z >= -p.w) \
        { \
            float x = p.x / p.w, y = p.y / p.w; \
            sx1 = min(sx1, x); \
            sy1 = min(sy1, y); \
            sx2 = max(sx2, x); \
            sy2 = max(sy2, y); \
        } \
    } while(0)
    vec4 v[8];
    sx1 = sy1 = 1;
    sx2 = sy2 = -1;
    camprojmatrix.transform(vec(bbmin.x, bbmin.y, bbmin.z), v[0]);
    ADDXYSCISSOR(v[0]);
    camprojmatrix.transform(vec(bbmax.x, bbmin.y, bbmin.z), v[1]);
    ADDXYSCISSOR(v[1]);
    camprojmatrix.transform(vec(bbmin.x, bbmax.y, bbmin.z), v[2]);
    ADDXYSCISSOR(v[2]);
    camprojmatrix.transform(vec(bbmax.x, bbmax.y, bbmin.z), v[3]);
    ADDXYSCISSOR(v[3]);
    camprojmatrix.transform(vec(bbmin.x, bbmin.y, bbmax.z), v[4]);
    ADDXYSCISSOR(v[4]);
    camprojmatrix.transform(vec(bbmax.x, bbmin.y, bbmax.z), v[5]);
    ADDXYSCISSOR(v[5]);
    camprojmatrix.transform(vec(bbmin.x, bbmax.y, bbmax.z), v[6]);
    ADDXYSCISSOR(v[6]);
    camprojmatrix.transform(vec(bbmax.x, bbmax.y, bbmax.z), v[7]);
    ADDXYSCISSOR(v[7]);
    if(sx1 > sx2 || sy1 > sy2) return false;
    loopi(8)
    {
        const vec4 &p = v[i];
        if(p.z >= -p.w) continue;
        loopj(3)
        {
            const vec4 &o = v[i^(1<<j)];
            if(o.z <= -o.w) continue;
#define INTERPXYSCISSOR(p, o) do { \
            float t = (p.z + p.w)/(p.z + p.w - o.z - o.w), \
                  w = p.w + t*(o.w - p.w), \
                  x = (p.x + t*(o.x - p.x))/w, \
                  y = (p.y + t*(o.y - p.y))/w; \
            sx1 = min(sx1, x); \
            sy1 = min(sy1, y); \
            sx2 = max(sx2, x); \
            sy2 = max(sy2, y); \
        } while(0)
            INTERPXYSCISSOR(p, o);
        }
    }
    sx1 = max(sx1, -1.0f);
    sy1 = max(sy1, -1.0f);
    sx2 = min(sx2, 1.0f);
    sy2 = min(sy2, 1.0f);
    return true;
}

bool calcspotscissor(const vec &origin, float radius, const vec &dir, int spot, const vec &spotx, const vec &spoty, float &sx1, float &sy1, float &sx2, float &sy2, float &sz1, float &sz2)
{
    float spotscale = radius * tan360(spot);
    vec up = vec(spotx).mul(spotscale), right = vec(spoty).mul(spotscale), center = vec(dir).mul(radius).add(origin);
#define ADDXYZSCISSOR(p) do { \
        if(p.z >= -p.w) \
        { \
            float x = p.x / p.w, y = p.y / p.w, z = p.z / p.w; \
            sx1 = min(sx1, x); \
            sy1 = min(sy1, y); \
            sz1 = min(sz1, z); \
            sx2 = max(sx2, x); \
            sy2 = max(sy2, y); \
            sz2 = max(sz2, z); \
        } \
    } while(0)
    vec4 v[5];
    sx1 = sy1 = sz1 = 1;
    sx2 = sy2 = sz2 = -1;
    camprojmatrix.transform(vec(center).sub(right).sub(up), v[0]);
    ADDXYZSCISSOR(v[0]);
    camprojmatrix.transform(vec(center).add(right).sub(up), v[1]);
    ADDXYZSCISSOR(v[1]);
    camprojmatrix.transform(vec(center).sub(right).add(up), v[2]);
    ADDXYZSCISSOR(v[2]);
    camprojmatrix.transform(vec(center).add(right).add(up), v[3]);
    ADDXYZSCISSOR(v[3]);
    camprojmatrix.transform(origin, v[4]);
    ADDXYZSCISSOR(v[4]);
    if(sx1 > sx2 || sy1 > sy2 || sz1 > sz2) return false;
    loopi(4)
    {
        const vec4 &p = v[i];
        if(p.z >= -p.w) continue;
        loopj(2)
        {
            const vec4 &o = v[i^(1<<j)];
            if(o.z <= -o.w) continue;
#define INTERPXYZSCISSOR(p, o) do { \
            float t = (p.z + p.w)/(p.z + p.w - o.z - o.w), \
                  w = p.w + t*(o.w - p.w), \
                  x = (p.x + t*(o.x - p.x))/w, \
                  y = (p.y + t*(o.y - p.y))/w; \
            sx1 = min(sx1, x); \
            sy1 = min(sy1, y); \
            sz1 = min(sz1, -1.0f); \
            sx2 = max(sx2, x); \
            sy2 = max(sy2, y); \
        } while(0)
            INTERPXYZSCISSOR(p, o);
        }
        if(v[4].z > -v[4].w) INTERPXYZSCISSOR(p, v[4]);
    }
    if(v[4].z < -v[4].w) loopj(4)
    {
        const vec4 &o = v[j];
        if(o.z <= -o.w) continue;
        INTERPXYZSCISSOR(v[4], o);
    }
    sx1 = max(sx1, -1.0f);
    sy1 = max(sy1, -1.0f);
    sz1 = max(sz1, -1.0f);
    sx2 = min(sx2, 1.0f);
    sy2 = min(sy2, 1.0f);
    sz2 = min(sz2, 1.0f);
    return true;
}

static GLuint screenquadvbo = 0;

static void setupscreenquad()
{
    if(!screenquadvbo)
    {
        glCheckError(glGenBuffers_(1, &screenquadvbo));
        gle::bindvbo(screenquadvbo);
        vec2 verts[4] = { vec2(1, -1), vec2(-1, -1), vec2(1, 1), vec2(-1, 1) };
        glCheckError(glBufferData_(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW));
        gle::clearvbo();
    }
}

static void cleanupscreenquad()
{
    if(screenquadvbo) { glCheckError(glDeleteBuffers_(1, &screenquadvbo)); screenquadvbo = 0; }
}

void screenquad()
{
    setupscreenquad();
    gle::bindvbo(screenquadvbo);
    gle::enablevertex();
    gle::vertexpointer(sizeof(vec2), (const vec2 *)0, GL_FLOAT, 2);
    glCheckError(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    gle::disablevertex();
    gle::clearvbo();
}

static LocalShaderParam screentexcoord[2] = { LocalShaderParam("screentexcoord0"), LocalShaderParam("screentexcoord1") };

static inline void setscreentexcoord(int i, float w, float h, float x = 0, float y = 0)
{
    screentexcoord[i].setf(w*0.5f, h*0.5f, x + w*0.5f, y + fabs(h)*0.5f);
}

void screenquad(float sw, float sh)
{
    setscreentexcoord(0, sw, sh);
    screenquad();
}

void screenquadflipped(float sw, float sh)
{
    setscreentexcoord(0, sw, -sh);
    screenquad();
}

void screenquad(float sw, float sh, float sw2, float sh2)
{
    setscreentexcoord(0, sw, sh);
    setscreentexcoord(1, sw2, sh2);
    screenquad();
}

void screenquadoffset(float x, float y, float w, float h)
{
    setscreentexcoord(0, w, h, x, y);
    screenquad();
}

void screenquadoffset(float x, float y, float w, float h, float x2, float y2, float w2, float h2)
{
    setscreentexcoord(0, w, h, x, y);
    setscreentexcoord(1, w2, h2, x2, y2);
    screenquad();
}

#define HUDQUAD(x1, y1, x2, y2, sx1, sy1, sx2, sy2) { \
    gle::defvertex(2); \
    gle::deftexcoord0(); \
    gle::begin(GL_TRIANGLE_STRIP); \
    gle::attribf(x2, y1); gle::attribf(sx2, sy1); \
    gle::attribf(x1, y1); gle::attribf(sx1, sy1); \
    gle::attribf(x2, y2); gle::attribf(sx2, sy2); \
    gle::attribf(x1, y2); gle::attribf(sx1, sy2); \
    gle::end(); \
}

void hudquad(float x, float y, float w, float h, float tx, float ty, float tw, float th)
{
    HUDQUAD(x, y, x+w, y+h, tx, ty, tx+tw, ty+th);
}

void debugquad(float x, float y, float w, float h, float tx, float ty, float tw, float th)
{
    HUDQUAD(x, y, x+w, y+h, tx, ty+th, tx+tw, ty);
}

VARR(fog, 16, 4000, 1000024);
CVARR(fogcolour, 0x8099B3);
VAR(fogoverlay, 0, 1, 1);

static float findsurface(int fogmat, const vec &v, int &abovemat)
{
    fogmat &= MATF_VOLUME;
    ivec o(v), co;
    int csize;
    do
    {
        cube &c = lookupcube(o, 0, co, csize);
        int mat = c.material&MATF_VOLUME;
        if(mat != fogmat)
        {
            abovemat = isliquid(mat) ? c.material : MAT_AIR;
            return o.z;
        }
        o.z = co.z + csize;
    }
    while(o.z < worldsize);
    abovemat = MAT_AIR;
    return worldsize;
}

static void blendfog(int fogmat, float below, float blend, float logblend, float &start, float &end, vec &fogc)
{
    switch(fogmat&MATF_VOLUME)
    {
        case MAT_WATER:
        {
            const bvec &wcol = getwatercolour(fogmat), &wdeepcol = getwaterdeepcolour(fogmat);
            int wfog = getwaterfog(fogmat), wdeep = getwaterdeep(fogmat);
            float deepfade = clamp(below/max(wdeep, wfog), 0.0f, 1.0f);
            vec color;
            color.lerp(wcol.tocolor(), wdeepcol.tocolor(), deepfade);
            fogc.add(vec(color).mul(blend));
            end += logblend*min(fog, max(wfog*2, 16));
            break;
        }

        case MAT_LAVA:
        {
            const bvec &lcol = getlavacolour(fogmat);
            int lfog = getlavafog(fogmat);
            fogc.add(lcol.tocolor().mul(blend));
            end += logblend*min(fog, max(lfog*2, 16));
            break;
        }

        default:
            fogc.add(fogcolour.tocolor().mul(blend));
            start += logblend*(fog+64)/8;
            end += logblend*fog;
            break;
    }
}

vec curfogcolor(0, 0, 0);

void setfogcolor(const vec &v)
{
    GLOBALPARAM(fogcolor, v);
}

void zerofogcolor()
{
    setfogcolor(vec(0, 0, 0));
}

void resetfogcolor()
{
    setfogcolor(curfogcolor);
}

FVAR(fogintensity, 0, 0.15f, 1);

float calcfogdensity(float dist)
{
    return log(fogintensity)/(M_LN2*dist);
}

FVAR(fogcullintensity, 0, 1e-3f, 1);

float calcfogcull()
{
    return log(fogcullintensity) / (M_LN2*calcfogdensity(fog - (fog+64)/8));
}

static void setfog(int fogmat, float below = 0, float blend = 1, int abovemat = MAT_AIR)
{
    float start = 0, end = 0;
    float logscale = 256, logblend = log(1 + (logscale - 1)*blend) / log(logscale);

    curfogcolor = vec(0, 0, 0);
    blendfog(fogmat, below, blend, logblend, start, end, curfogcolor);
    if(blend < 1) blendfog(abovemat, 0, 1-blend, 1-logblend, start, end, curfogcolor);
    curfogcolor.mul(ldrscale);

    GLOBALPARAM(fogcolor, curfogcolor);

    float fogdensity = calcfogdensity(end-start);
    GLOBALPARAMF(fogdensity, fogdensity, 1/exp(M_LN2*start*fogdensity));
}

static void blendfogoverlay(int fogmat, float below, float blend, vec &overlay)
{
    float maxc;
    switch(fogmat&MATF_VOLUME)
    {
        case MAT_WATER:
        {
            const bvec &wcol = getwatercolour(fogmat), &wdeepcol = getwaterdeepcolour(fogmat);
            int wfog = getwaterfog(fogmat), wdeep = getwaterdeep(fogmat);
            float deepfade = clamp(below/max(wdeep, wfog), 0.0f, 1.0f);
            vec color = vec(wcol.r, wcol.g, wcol.b).lerp(vec(wdeepcol.r, wdeepcol.g, wdeepcol.b), deepfade);
            overlay.add(color.div(min(32.0f + max(color.r, max(color.g, color.b))*7.0f/8.0f, 255.0f)).max(0.4f).mul(blend));
            break;
        }

        case MAT_LAVA:
        {
            const bvec &lcol = getlavacolour(fogmat);
            maxc = max(lcol.r, max(lcol.g, lcol.b));
            overlay.add(vec(lcol.r, lcol.g, lcol.b).div(min(32.0f + maxc*7.0f/8.0f, 255.0f)).max(0.4f).mul(blend));
            break;
        }

        default:
            overlay.add(blend);
            break;
    }
}

void drawfogoverlay(int fogmat, float fogbelow, float fogblend, int abovemat)
{
    SETSHADER(fogoverlay);

    glCheckError(glEnable(GL_BLEND));
    glCheckError(glBlendFunc(GL_ZERO, GL_SRC_COLOR));
    vec overlay(0, 0, 0);
    blendfogoverlay(fogmat, fogbelow, fogblend, overlay);
    blendfogoverlay(abovemat, 0, 1-fogblend, overlay);

    gle::color(overlay);
    screenquad();

    glCheckError(glDisable(GL_BLEND));
}

int drawtex = 0;

GLuint minimaptex = 0;
vec minimapcenter(0, 0, 0), minimapradius(0, 0, 0), minimapscale(0, 0, 0);

void clearminimap()
{
    if(minimaptex) { glCheckError(glDeleteTextures(1, &minimaptex)); minimaptex = 0; }
}

VARR(minimapheight, 0, 0, 2<<16);
CVARR(minimapcolour, 0);
VARR(minimapclip, 0, 0, 1);
VARFP(minimapsize, 7, 8, 10, { if(minimaptex) drawminimap(); });
VARFP(showminimap, 0, 1, 1, { if(minimaptex) drawminimap(); });
CVARFP(nominimapcolour, 0x101010, { if(minimaptex && !showminimap) drawminimap(); });

void bindminimap()
{
    glCheckError(glBindTexture(GL_TEXTURE_2D, minimaptex));
}

void clipminimap(ivec &bbmin, ivec &bbmax, cube *c = worldroot, const ivec &co = ivec(0, 0, 0), int size = worldsize>>1)
{
    loopi(8)
    {
        ivec o(i, co, size);
        if(c[i].children) clipminimap(bbmin, bbmax, c[i].children, o, size>>1);
        else if(!isentirelysolid(c[i]) && (c[i].material&MATF_CLIP)!=MAT_CLIP)
        {
            loopk(3) bbmin[k] = min(bbmin[k], o[k]);
            loopk(3) bbmax[k] = max(bbmax[k], o[k] + size);
        }
    }
}

void drawminimap()
{
//    if(!game::needminimap()) { clearminimap(); return; }
//
//    if(!showminimap)
//    {
//        if(!minimaptex){
//            glCheckError(glGenTextures(1, &minimaptex));
//        }
//        createtexture(minimaptex, 1, 1, nominimapcolour.v, 3, 0, GL_RGB, GL_TEXTURE_2D);
//        return;
//    }
//
//    GLERROR;
//    renderprogress(0, "generating mini-map...", !renderedframe);
//
//    drawtex = DRAWTEX_MINIMAP;
//
//    GLERROR;
//    gl_setupframe(true);
//
//    int size = 1<<minimapsize, sizelimit = min(hwtexsize, min(gw, gh));
//    while(size > sizelimit) size /= 2;
//    if(!minimaptex){
//        glCheckError(glGenTextures(1, &minimaptex));
//    }
//
//    ivec bbmin(worldsize, worldsize, worldsize), bbmax(0, 0, 0);
//    loopv(valist)
//    {
//        vtxarray *va = valist[i];
//        loopk(3)
//        {
//            if(va->geommin[k]>va->geommax[k]) continue;
//            bbmin[k] = min(bbmin[k], va->geommin[k]);
//            bbmax[k] = max(bbmax[k], va->geommax[k]);
//        }
//    }
//    if(minimapclip)
//    {
//        ivec clipmin(worldsize, worldsize, worldsize), clipmax(0, 0, 0);
//        clipminimap(clipmin, clipmax);
//        loopk(2) bbmin[k] = max(bbmin[k], clipmin[k]);
//        loopk(2) bbmax[k] = min(bbmax[k], clipmax[k]);
//    }
//
//    minimapradius = vec(bbmax).sub(vec(bbmin)).mul(0.5f);
//    minimapcenter = vec(bbmin).emplace_back(minimapradius);
//    minimapradius.x = minimapradius.y = max(minimapradius.x, minimapradius.y);
//    minimapscale = vec((0.5f - 1.0f/size)/minimapradius.x, (0.5f - 1.0f/size)/minimapradius.y, 1.0f);
//
//    MovableEntity *oldcamera = activeCamera;
//    static MovableEntity cmcamera;
//    cmcamera = *player;
////    cmcamera.ent_type = ENT_CAMERA;
//    cmcamera.o = vec(minimapcenter.x, minimapcenter.y, minimapheight > 0 ? minimapheight : minimapcenter.z + minimapradius.z + 1);
//    cmcamera.d.x = 0;
//    cmcamera.d.y = -90;
//    cmcamera.d.z = 0;
//    activeCamera = &cmcamera;
//    setviewcell(vec(-1, -1, -1));
//
//    float oldldrscale = ldrscale, oldldrscaleb = ldrscaleb;
//    int oldfarplane = farplane, oldvieww = vieww, oldviewh = viewh;
//    farplane = worldsize*2;
//    vieww = viewh = size;
//
//    float zscale = max(float(minimapheight), minimapcenter.z + minimapradius.z + 1) + 1;
//
//    projmatrix.ortho(-minimapradius.x, minimapradius.x, -minimapradius.y, minimapradius.y, 0, 2*zscale);
//    setcamprojmatrix();
//
//    glCheckError(glEnable(GL_CULL_FACE));
//    glCheckError(glEnable(GL_DEPTH_TEST));
//
//    xtravertsva = xtraverts = glde = gbatches = vtris = vverts = 0;
//    flipqueries();
//
//    ldrscale = 1;
//    ldrscaleb = ldrscale/255;
//
//    visiblecubes(false);
//
//    rendergbuffer();
//
//    rendershadowatlas();
//
//    shademinimap(minimapcolour.tocolor().mul(ldrscale));
//
//    if(minimapheight > 0 && minimapheight < minimapcenter.z + minimapradius.z)
//    {
//        activeCamera->o.z = minimapcenter.z + minimapradius.z + 1;
//        projmatrix.ortho(-minimapradius.x, minimapradius.x, -minimapradius.y, minimapradius.y, -zscale, zscale);
//        setcamprojmatrix();
//        rendergbuffer(false);
//        shademinimap();
//    }
//
//    glCheckError(glDisable(GL_DEPTH_TEST));
//    glCheckError(glDisable(GL_CULL_FACE));
//
//    farplane = oldfarplane;
//    vieww = oldvieww;
//    viewh = oldviewh;
//    ldrscale = oldldrscale;
//    ldrscaleb = oldldrscaleb;
//
//    activeCamera = oldcamera;
//    drawtex = 0;
//
//    createtexture(minimaptex, size, size, NULL, 3, 1, GL_RGBA, GL_TEXTURE_2D);
//    glCheckError(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
//    glCheckError(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
//    GLfloat border[4] = { minimapcolour.x/255.0f, minimapcolour.y/255.0f, minimapcolour.z/255.0f, 1.0f };
//    glCheckError(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border));
//    glCheckError(glBindTexture(GL_TEXTURE_2D, 0));
//
//    GLuint fbo = 0;
//    glCheckError(glGenFramebuffers_(1, &fbo));
//    glCheckError(glBindFramebuffer_(GL_FRAMEBUFFER, fbo));
//    glCheckError(glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, minimaptex, 0));
//    copyhdr(size, size, fbo);
//    glCheckError(glBindFramebuffer_(GL_FRAMEBUFFER, 0));
//    glCheckError(glDeleteFramebuffers_(1, &fbo));
//
//    glCheckError(glViewport(0, 0, hudw, hudh));
}

void drawcubemap(int size, const vec &o, float yaw, float pitch, const cubemapside &side, bool onlysky)
{
//    drawtex = DRAWTEX_ENVMAP;
//
//    MovableEntity *oldcamera = activeCamera;
//    static MovableEntity cmcamera;
//    cmcamera = *player;
////    cmcamera.ent_type = ENT_CAMERA;
//    cmcamera.o = o;
//    cmcamera.d.x = yaw;
//    cmcamera.d.y = pitch;
//    cmcamera.d.z = 0;
//    activeCamera = &cmcamera;
//    setviewcell(activeCamera->o);
//
//    float fogmargin = 1 + WATER_AMPLITUDE + nearplane;
//    int fogmat = lookupmaterial(vec(activeCamera->o.x, activeCamera->o.y, activeCamera->o.z - fogmargin))&(MATF_VOLUME|MATF_INDEX), abovemat = MAT_AIR;
//    float fogbelow = 0;
//    if(isliquid(fogmat&MATF_VOLUME))
//    {
//        float z = findsurface(fogmat, vec(activeCamera->o.x, activeCamera->o.y, activeCamera->o.z - fogmargin), abovemat) - WATER_OFFSET;
//        if(activeCamera->o.z < z + fogmargin)
//        {
//            fogbelow = z - activeCamera->o.z;
//        }
//        else fogmat = abovemat;
//    }
//    else fogmat = MAT_AIR;
//    setfog(abovemat);
//
//    float oldaspect = aspect, oldfovy = fovy, oldfov = curfov, oldldrscale = ldrscale, oldldrscaleb = ldrscaleb;
//    int oldfarplane = farplane, oldvieww = vieww, oldviewh = viewh;
//    curfov = fovy = 90;
//    aspect = 1;
//    farplane = worldsize*2;
//    vieww = viewh = size;
//    projmatrix.perspective(fovy, aspect, nearplane, farplane);
//    setcamprojmatrix();
//
//    glCheckError(glEnable(GL_CULL_FACE));
//    glCheckError(glEnable(GL_DEPTH_TEST));
//
//    xtravertsva = xtraverts = glde = gbatches = vtris = vverts = 0;
//    flipqueries();
//
//    ldrscale = 1;
//    ldrscaleb = ldrscale/255;
//
//    visiblecubes();
//
//    if(onlysky)
//    {
//        preparegbuffer();
//        GLERROR;
//
//        shadesky();
//        GLERROR;
//    }
//    else
//    {
//        rendergbuffer();
//        GLERROR;
//
//        renderradiancehints();
//        GLERROR;
//
//        rendershadowatlas();
//        GLERROR;
//
//        shadegbuffer();
//        GLERROR;
//
//        if(fogmat)
//        {
//            setfog(fogmat, fogbelow, 1, abovemat);
//
//            renderwaterfog(fogmat, fogbelow);
//
//            setfog(fogmat, fogbelow, clamp(fogbelow, 0.0f, 1.0f), abovemat);
//        }
//
//        rendertransparent();
//        GLERROR;
//    }
//
//    glCheckError(glDisable(GL_DEPTH_TEST));
//    glCheckError(glDisable(GL_CULL_FACE));
//
//    aspect = oldaspect;
//    fovy = oldfovy;
//    curfov = oldfov;
//    farplane = oldfarplane;
//    vieww = oldvieww;
//    viewh = oldviewh;
//    ldrscale = oldldrscale;
//    ldrscaleb = oldldrscaleb;
//
//    activeCamera = oldcamera;
//    drawtex = 0;
}

VAR(modelpreviewfov, 10, 20, 100);
VAR(modelpreviewpitch, -90, -15, 90);

//namespace modelpreview
//{
//    MovableEntity *oldcamera;
//    MovableEntity camera;
//
//    float oldaspect, oldfovy, oldfov, oldldrscale, oldldrscaleb;
//    int oldfarplane, oldvieww, oldviewh;
//
//    int x, y, w, h;
//    bool background, scissor;
//
//    void start(int x, int y, int w, int h, bool background, bool scissor)
//    {
//        modelpreview::x = x;
//        modelpreview::y = y;
//        modelpreview::w = w;
//        modelpreview::h = h;
//        modelpreview::background = background;
//        modelpreview::scissor = scissor;
//
//        setupgbuffer();
//
//        useshaderbyname("modelpreview");
//
//        drawtex = DRAWTEX_MODELPREVIEW;
//
//        oldcamera = activeCamera;
//        camera = *activeCamera;
////        camera.et_type = ET_GAMESPECIFIC;
////        camera.ent_type = ENT_CAMERA;
////        camera.game_type = GAMEENTITY;
//        camera.o = vec(0, 0, 0);
//        camera.d.x = 0;
//        camera.d.y = modelpreviewpitch;
//        camera.d.z = 0;
//        activeCamera = &camera;
//
//        oldaspect = aspect;
//        oldfovy = fovy;
//        oldfov = curfov;
//        oldldrscale = ldrscale;
//        oldldrscaleb = ldrscaleb;
//        oldfarplane = farplane;
//        oldvieww = vieww;
//        oldviewh = viewh;
//
//        aspect = w/float(h);
//        fovy = modelpreviewfov;
//        curfov = 2*atan2(tan(fovy/2*RAD), 1/aspect)/RAD;
//        farplane = 1024;
//        vieww = min(gw, w);
//        viewh = min(gh, h);
//        ldrscale = 1;
//        ldrscaleb = ldrscale/255;
//
//        projmatrix.perspective(fovy, aspect, nearplane, farplane);
//        setcamprojmatrix();
//
//        glCheckError(glEnable(GL_CULL_FACE));
//        glCheckError(glEnable(GL_DEPTH_TEST));
//
//        preparegbuffer();
//    }
//
//    void end()
//    {
//        rendermodelbatches();
//
//        glCheckError(glDisable(GL_DEPTH_TEST));
//        glCheckError(glDisable(GL_CULL_FACE));
//
//        shademodelpreview(x, y, w, h, background, scissor);
//
//        aspect = oldaspect;
//        fovy = oldfovy;
//        curfov = oldfov;
//        farplane = oldfarplane;
//        vieww = oldvieww;
//        viewh = oldviewh;
//        ldrscale = oldldrscale;
//        ldrscaleb = oldldrscaleb;
//
//        activeCamera = oldcamera;
//        drawtex = 0;
//    }
//}

vec calcmodelpreviewpos(const vec &radius, float &yaw)
{
    assert(Camera::GetActiveCamera());

    yaw = fmod(lastmillis/10000.0f*360.0f, 360.0f);
    float dist = max(radius.magnitude2()/aspect, radius.magnitude())/sinf(fovy/2*RAD);
    return vec(0, dist, 0).rotate_around_x(Camera::GetActiveCamera()->d.y*RAD);
}

int xtraverts, xtravertsva;

void gl_drawview()
{
    GLuint scalefbo = shouldscale();
    if(scalefbo) { vieww = gw; viewh = gh; }

    const auto& activeCamera = Camera::GetActiveCamera();

    float fogmargin = 1 + WATER_AMPLITUDE + nearplane;
    int fogmat = lookupmaterial(vec(activeCamera->o.x, activeCamera->o.y, activeCamera->o.z - fogmargin))&(MATF_VOLUME|MATF_INDEX), abovemat = MAT_AIR;
    float fogbelow = 0;
    if(isliquid(fogmat&MATF_VOLUME))
    {
        float z = findsurface(fogmat, vec(activeCamera->o.x, activeCamera->o.y, activeCamera->o.z - fogmargin), abovemat) - WATER_OFFSET;
        if(activeCamera->o.z < z + fogmargin)
        {
            fogbelow = z - activeCamera->o.z;
        }
        else fogmat = abovemat;
    }
    else fogmat = MAT_AIR;
    setfog(abovemat);
    setfog(fogmat, fogbelow, 1, abovemat);

    farplane = worldsize*2;

    projmatrix.perspective(fovy, aspect, nearplane, farplane);
    setcamprojmatrix();

    glCheckError(glEnable(GL_CULL_FACE));
    glCheckError(glEnable(GL_DEPTH_TEST));

    ldrscale = 0.5f;
    ldrscaleb = ldrscale/255;

    visiblecubes(false);

    if(wireframe && editmode){
#ifndef OPEN_GL_ES
        glCheckError(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
#endif
    }

    rendergbuffer();

    if(wireframe && editmode){
#ifndef OPEN_GL_ES
        glCheckError(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
#endif
    }
    else if(limitsky() && editmode) renderexplicitsky(true);

    renderao();
    GLERROR;

    // render avatar after AO to avoid weird contact shadows
    renderavatar();
    GLERROR;
 
    // render grass after AO to avoid disturbing shimmering patterns
    generategrass();
    rendergrass();
    GLERROR;

    glFlush();

    renderradiancehints();
    GLERROR;

    rendershadowatlas();
    GLERROR;

    shadegbuffer();
    GLERROR;

    if(fogmat)
    {
        setfog(fogmat, fogbelow, 1, abovemat);

        renderwaterfog(fogmat, fogbelow);

        setfog(fogmat, fogbelow, clamp(fogbelow, 0.0f, 1.0f), abovemat);
    }

    rendertransparent();
    GLERROR;

    if(fogmat) setfog(fogmat, fogbelow, 1, abovemat);

    rendervolumetric();
    GLERROR;

    if(editmode)
    {
        extern int outline;
        if(!wireframe && outline) renderoutline();
        GLERROR;
        rendereditmaterials();
        GLERROR;
        renderparticles();
        GLERROR;

        extern int hidehud;
        if(!hidehud)
        {
            glCheckError(glDepthMask(GL_FALSE));
            renderblendbrush();
            rendereditcursor();
            game::rendergame(game::RenderPass::Edit);
            glCheckError(glDepthMask(GL_TRUE));
        }
    }

    glCheckError(glDisable(GL_CULL_FACE));
    glCheckError(glDisable(GL_DEPTH_TEST));

    if(fogoverlay && fogmat != MAT_AIR) drawfogoverlay(fogmat, fogbelow, clamp(fogbelow, 0.0f, 1.0f), abovemat);

    doaa(setuppostfx(vieww, viewh, scalefbo), processhdr);
    renderpostfx(scalefbo);
    if(scalefbo) doscale();
}

void gl_drawmainmenu()
{
    renderbackground(NULL, NULL, NULL, NULL, true);
}

VARNP(damagecompass, usedamagecompass, 0, 1, 1);
VARP(damagecompassfade, 1, 1000, 10000);
VARP(damagecompasssize, 1, 30, 100);
VARP(damagecompassalpha, 1, 25, 100);
VARP(damagecompassmin, 1, 25, 1000);
VARP(damagecompassmax, 1, 200, 1000);

float damagedirs[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

void damagecompass(int n, const vec &loc)
{
    const auto& activeCamera = Camera::GetActiveCamera();
    assert(activeCamera);

    if(!usedamagecompass || Application::Instance().GetAppState().Minimized) return;
    vec delta(loc);
    delta.sub(activeCamera->o);
    float yaw = 0, pitch;
    if(delta.magnitude() > 4)
    {
        vectoyawpitch(delta, yaw, pitch);
        yaw -= activeCamera->d.x;
    }
    if(yaw >= 360) yaw = fmod(yaw, 360);
    else if(yaw < 0) yaw = 360 - fmod(-yaw, 360);
    int dir = (int(yaw+22.5f)%360)/45;
    damagedirs[dir] += max(n, damagecompassmin)/float(damagecompassmax);
    if(damagedirs[dir]>1) damagedirs[dir] = 1;

}
void drawdamagecompass(int w, int h)
{
    hudnotextureshader->set();

    int dirs = 0;
    float size = damagecompasssize/100.0f*min(h, w)/2.0f;
    loopi(8) if(damagedirs[i]>0)
    {
        if(!dirs)
        {
            glCheckError(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            gle::colorf(1, 0, 0, damagecompassalpha/100.0f);
            gle::defvertex();
            gle::begin(GL_TRIANGLES);
        }
        dirs++;

        float logscale = 32,
              scale = log(1 + (logscale - 1)*damagedirs[i]) / log(logscale),
              offset = -size/2.0f-min(h, w)/4.0f;
        matrix4x3 m;
        m.identity();
        m.settranslation(w/2, h/2, 0);
        m.rotate_around_z(i*45*RAD);
        m.translate(0, offset, 0);
        m.scale(size*scale);

        gle::attrib(m.transform(vec2(1, 1)));
        gle::attrib(m.transform(vec2(-1, 1)));
        gle::attrib(m.transform(vec2(0, 0)));

        // fade in log space so short blips don't disappear too quickly
        scale -= float(curtime)/damagecompassfade;
        damagedirs[i] = scale > 0 ? (pow(logscale, scale) - 1) / (logscale - 1) : 0;
    }
    if(dirs) gle::end();
}

int damageblendmillis = 0;

VARFP(damagescreen, 0, 1, 1, { if(!damagescreen) damageblendmillis = 0; });
VARP(damagescreenfactor, 1, 75, 100);
VARP(damagescreenalpha, 1, 45, 100);
VARP(damagescreenfade, 0, 1000, 1000);
VARP(damagescreenmin, 1, 10, 1000);
VARP(damagescreenmax, 1, 100, 1000);

void damageblend(int n)
{
    if(!damagescreen || Application::Instance().GetAppState().Minimized) return;
    if(lastmillis > damageblendmillis) damageblendmillis = lastmillis;
    damageblendmillis += clamp(n, damagescreenmin, damagescreenmax)*damagescreenfactor;
}

void drawdamagescreen(int w, int h)
{
    if(lastmillis >= damageblendmillis) return;

    hudshader->set();

    static Texture *damagetex = NULL;
    if(!damagetex) damagetex = textureload("media/interface/hud/damage.png", 3);

    glCheckError(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
    glCheckError(glBindTexture(GL_TEXTURE_2D, damagetex->id));
    float fade = damagescreenalpha/100.0f;
    if(damageblendmillis - lastmillis < damagescreenfade)
        fade *= float(damageblendmillis - lastmillis)/damagescreenfade;
    gle::colorf(fade, fade, fade, fade);

    hudquad(0, 0, w, h);
}

VAR(hidestats, 0, 0, 1);
VAR(hidehud, 0, 0, 1);

VARP(crosshairsize, 0, 15, 50);
VARP(cursorsize, 0, 15, 30);
VARP(crosshairfx, 0, 1, 1);
VARP(crosshaircolors, 0, 1, 1);

#define MAXCROSSHAIRS 4
static Texture *crosshairs[MAXCROSSHAIRS] = { NULL, NULL, NULL, NULL };

void loadcrosshair(const char *name, int i)
{
    if(i < 0 || i >= MAXCROSSHAIRS) return;
    crosshairs[i] = name ? textureload(name, 3, true) : notexture;
    if(crosshairs[i] == notexture)
    {
        name = game::defaultcrosshair(i);
        if(!name) name = "media/interface/crosshair/default.png";
        crosshairs[i] = textureload(name, 3, true);
    }
}

SCRIPTEXPORT_AS(loadcrosshair) void loadcrosshair_(const char *name, int *i)
{
    loadcrosshair(name, *i);
}

SCRIPTEXPORT void getcrosshair(int *i)
{
    const char *name = "";
    if(*i >= 0 && *i < MAXCROSSHAIRS)
    {
        name = crosshairs[*i] ? crosshairs[*i]->name : game::defaultcrosshair(*i);
        if(!name) name = "media/interface/crosshair/default.png";
    }
    result(name);
}

void writecrosshairs(stream *f)
{
    loopi(MAXCROSSHAIRS) if(crosshairs[i] && crosshairs[i]!=notexture)
        f->printf("loadcrosshair %s %d\n", escapecubestr(crosshairs[i]->name), i);
    f->printf("\n");
}

void drawcrosshair(int w, int h)
{
    if (!Application::Instance().GetInput().IsMouseGrabbed())
        return;

    if(hidehud || mainmenu) return; //(hidehud || player->state==CS_SPECTATOR || player->state==CS_DEAD)) return;

    vec color(1, 1, 1);
    // TODO: Change crosshair over here?
    float cx = 0.5f, cy = 0.5f, chsize;
    Texture *crosshair;

    int index = game::selectcrosshair(color);
    if(index < 0) return;
    if(!crosshairfx) index = 0;
    if(!crosshairfx || !crosshaircolors) color = vec(1, 1, 1);
    crosshair = crosshairs[index];
    if(!crosshair)
    {
        loadcrosshair(NULL, index);
        crosshair = crosshairs[index];
    }
    chsize = crosshairsize*w/900.0f;

    if(crosshair->type&Texture::ALPHA){
        glCheckError(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    }
    else{
        glCheckError(glBlendFunc(GL_ONE, GL_ONE));
    }
    hudshader->set();
    gle::color(color);
    float x = cx*w - chsize/2.0f;
    float y = cy*h - chsize/2.0f;
    glCheckError(glBindTexture(GL_TEXTURE_2D, crosshair->id));

    hudquad(x, y, chsize, chsize);
}

VARP(wallclock, 0, 0, 1);
VARP(wallclock24, 0, 0, 1);
VARP(wallclocksecs, 0, 0, 1);

static time_t walltime = 0;

VARP(showfps, 0, 1, 1);
VARP(showfpsrange, 0, 0, 1);
VAR(statrate, 1, 200, 1000);

FVARP(conscale, 1e-3f, 0.33f, 1e3f);

void resethudshader()
{
    hudshader->set();
    gle::colorf(1, 1, 1);
}

void gl_drawhud()
{
    int w = hudw, h = hudh;
    if(forceaspect) w = int(ceil(h*forceaspect));

    gettextres(w, h);

    hudmatrix.ortho(0, w, h, 0, -1, 1);
    resethudmatrix();
    resethudshader();

    pushfont();
    setfont("default_outline");

    debuglights();

    glCheckError(glEnable(GL_BLEND));

    debugparticles();

    if(!mainmenu)
    {
        drawdamagescreen(w, h);
        drawdamagecompass(w, h);
    }

    float conw = w/conscale, conh = h/conscale, abovehud = conh - FONTH;
    if(!hidehud && !mainmenu)
    {
        if(!hidestats)
        {
            pushhudscale(conscale);

            int roffset = 0;
            if(showfps)
            {
                static int lastfps = 0, prevfps[3] = { 0, 0, 0 }, curfps[3] = { 0, 0, 0 };
                if(totalmillis - lastfps >= statrate)
                {
                    memcpy(prevfps, curfps, sizeof(prevfps));
                    lastfps = totalmillis - (totalmillis%statrate);
                }
                int nextfps[3];
                getfps(nextfps[0], nextfps[1], nextfps[2]);
                loopi(3) if(prevfps[i]==curfps[i]) curfps[i] = nextfps[i];
                if(showfpsrange) draw_textf("fps %d+%d-%d", conw-7*FONTH, conh-FONTH*3/2, curfps[0], curfps[1], curfps[2]);
                else draw_textf("fps %d", conw-5*FONTH, conh-FONTH*3/2, curfps[0]);
                roffset += FONTH;
            }

            printtimers(conw, conh);

            if(wallclock)
            {
                if(!walltime) { walltime = time(NULL); walltime -= totalmillis/1000; if(!walltime) walltime++; }
                time_t walloffset = walltime + totalmillis/1000;
                struct tm *localvals = localtime(&walloffset);
                static cubestr buf;
                if(localvals && strftime(buf, sizeof(buf), wallclocksecs ? (wallclock24 ? "%H:%M:%S" : "%I:%M:%S%p") : (wallclock24 ? "%H:%M" : "%I:%M%p"), localvals))
                {
                    // hack because not all platforms (windows) support %P lowercase option
                    // also strip leading 0 from 12 hour time
                    char *dst = buf;
                    const char *src = &buf[!wallclock24 && buf[0]=='0' ? 1 : 0];
                    while(*src) *dst++ = tolower(*src++);
                    *dst++ = '\0';
                    draw_text(buf, conw-5*FONTH, conh-FONTH*3/2-roffset);
                    roffset += FONTH;
                }
            }

            pophudmatrix();
        }

        if(!editmode)
        {
            resethudshader();
            glCheckError(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            game::gameplayhud(w, h);
            abovehud = min(abovehud, conh*game::abovegameplayhud(w, h));
        }

        rendertexturepanel(w, h);
    }

    abovehud = min(abovehud, conh*UI::abovehud());

    pushhudscale(conscale);
    abovehud -= rendercommand(FONTH/2, abovehud - FONTH/2, conw-FONTH);

    pophudmatrix();

    drawcrosshair(w, h);

    glCheckError(glDisable(GL_BLEND));

    popfont();

    if(frametimer)
    {
        glFinish();
        framemillis = getclockmillis() - totalmillis;
    }
}

int renderw = 0, renderh = 0, hudw = 0, hudh = 0;

void gl_setupframe(bool force)
{
    Application::Instance().GetWindow().GetContext().GetFramebufferSize(renderw, renderh);
    Application::Instance().GetWindow().GetContext().GetFramebufferSize(hudw, hudh);
    if(!force) return;
    setuplights();
}

void gl_drawframe()
{
    synctimers();
    xtravertsva = xtraverts = glde = gbatches = vtris = vverts = 0;
    flipqueries();
    aspect = forceaspect ? forceaspect : hudw/float(hudh);
    fovy = 2*atan2(tan(curfov/2*RAD), aspect)/RAD;
    vieww = hudw;
    viewh = hudh;
    if(mainmenu) gl_drawmainmenu();
    else gl_drawview();
    UI::render();
    gl_drawhud();
    
    game::rendergame(game::RenderPass::Gui);
}

void cleanupgl()
{
    clearminimap();
    cleanuptimers();
    cleanupscreenquad();
    gle::cleanup();
}

//#ifdef DEBUG

GLenum DebugOpenGL(const char *expression, const char *file, int line)
{
    std::string path = file;
    std::string filebase = path.substr(path.find_last_of("/\\") + 1);

    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string errorName;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  errorName = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 errorName = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             errorName = "INVALID_OPERATION"; break;
            // case GL_STACK_OVERFLOW:                errorName = "STACK_OVERFLOW"; break;
            // case GL_STACK_UNDERFLOW:               errorName = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 errorName = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: errorName = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        conoutf(CON_ERROR, "OpenGL error: %s:%d: %s %s", filebase.c_str(), line, expression, errorName.c_str());
    }

    return errorCode;
}

//#endif
