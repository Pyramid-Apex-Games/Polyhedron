#pragma once

#include <shared/types.h>

extern float matliquidsx1, matliquidsy1, matliquidsx2, matliquidsy2;
extern float matsolidsx1, matsolidsy1, matsolidsx2, matsolidsy2;
extern float matrefractsx1, matrefractsy1, matrefractsx2, matrefractsy2;
extern uint matliquidtiles[LIGHTTILE_MAXH], matsolidtiles[LIGHTTILE_MAXH];
extern vector<materialsurface> editsurfs, glasssurfs[4], watersurfs[4], waterfallsurfs[4], lavasurfs[4], lavafallsurfs[4];
extern const vec matnormals[6];

extern int showmat;

int findmaterial(const char *name);
const char *findmaterialname(int mat);
const char *getmaterialdesc(int mat, const char *prefix = "");
void genmatsurfs(const cube &c, const ivec &co, int size, vector<materialsurface> &matsurfs);
void calcmatbb(vtxarray *va, const ivec &co, int size, vector<materialsurface> &matsurfs);
int optimizematsurfs(materialsurface *matbuf, int matsurfs);
void setupmaterials(int start = 0, int len = 0);
int findmaterials();
void rendermaterialmask();
void renderliquidmaterials();
void rendersolidmaterials();
void rendereditmaterials();
void renderminimapmaterials();
int visiblematerial(const cube &c, int orient, const ivec &co, int size, ushort matmask = MATF_VOLUME);