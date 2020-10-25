#pragma once
#include "shared/geom/ivec.h"
#include "shared/geom/vec.h"

struct undoblock;
struct block3;

extern vector<ushort> texmru;

void cancelsel();
void rendertexturepanel(int w, int h);
void addundo(undoblock *u);
void commitchanges(bool force = false);
void changed(const ivec &bbmin, const ivec &bbmax, bool commit = true);
void changed(const block3 &sel, bool commit = true);
void rendereditcursor();
void tryedit();

void renderprefab(const char *name, const vec &o, float yaw, float pitch, float roll, float size = 1, const vec &color = vec(1, 1, 1));
void previewprefab(const char *name, const vec &color);
void cleanupprefabs();