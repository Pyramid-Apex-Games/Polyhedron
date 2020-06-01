#pragma once

#include "engine/engine.h"
#include <shared/types.h>
#include <shared/geom/ivec.h>
#include <shared/geom/vec.h>
#include <shared/entities/coreentity.h>

struct vtxarray;
struct occludequery;

extern int oqfrags;
extern float alphafrontsx1, alphafrontsx2, alphafrontsy1, alphafrontsy2, alphabacksx1, alphabacksx2, alphabacksy1, alphabacksy2, alpharefractsx1, alpharefractsx2, alpharefractsy1, alpharefractsy2;
extern uint alphatiles[LIGHTTILE_MAXH];
extern vtxarray *visibleva;

void visiblecubes(bool cull = true);
void setvfcP(const vec &bbmin = vec(-1, -1, -1), const vec &bbmax = vec(1, 1, 1));
void savevfcP();
void restorevfcP();
void rendergeom();
int findalphavas();
void renderrefractmask();
void renderalphageom(int side);
void rendermapmodels();
void renderoutline();
void cleanupva();

bool isfoggedsphere(float rad, const vec &cv);
int isvisiblesphere(float rad, const vec &cv);
int isvisiblebb(const ivec &bo, const ivec &br);
bool bboccluded(const ivec &bo, const ivec &br);

extern int deferquery;

void flipqueries();
occludequery *newquery(void *owner);
void startquery(occludequery *query);
void endquery(occludequery *query);
bool checkquery(occludequery *query, bool nowait = false);
void resetqueries();
int getnumqueries();
void startbb(bool mask = true);
void endbb(bool mask = true);
void drawbb(const ivec &bo, const ivec &br);

void renderdecals();

struct shadowmesh;

void clearshadowmeshes();
void genshadowmeshes();
shadowmesh *findshadowmesh(int idx, entities::classes::CoreEntity *e);
void rendershadowmesh(shadowmesh *m);