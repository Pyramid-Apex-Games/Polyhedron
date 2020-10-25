#pragma once

extern int blendpaintmode;

struct BlendMapCache;

BlendMapCache *newblendmapcache();
void freeblendmapcache(BlendMapCache *&cache);
bool setblendmaporigin(BlendMapCache *cache, const ivec &o, int size);
bool hasblendmap(BlendMapCache *cache);
uchar lookupblendmap(BlendMapCache *cache, const vec &pos);
void resetblendmap();
void enlargeblendmap();
void shrinkblendmap(int octant);
void optimizeblendmap();
void stoppaintblendmap();
void trypaintblendmap();
void renderblendbrush(GLuint tex, float x, float y, float w, float h);
void renderblendbrush();
bool loadblendmap(stream *f, int info);
void saveblendmap(stream *f);
uchar shouldsaveblendmap();
bool usesblendmap(int x1 = 0, int y1 = 0, int x2 = worldsize, int y2 = worldsize);
int calcblendlayer(int x1, int y1, int x2, int y2);
void updateblendtextures(int x1 = 0, int y1 = 0, int x2 = worldsize, int y2 = worldsize);
void bindblendtexture(const ivec &p);
void clearblendtextures();
void cleanupblendmap();