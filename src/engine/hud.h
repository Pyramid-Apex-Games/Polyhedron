#pragma once
#include "shared/geom/matrix4.h"

extern matrix4 hudmatrix;

void resethudmatrix();
void pushhudmatrix();
void flushhudmatrix(bool flushparams = true);
void pophudmatrix(bool flush = true, bool flushparams = true);
void pushhudscale(float sx, float sy = 0.0f);
void resethudshader();
void pushhudtranslate(float tx, float ty, float sx = 0, float sy = 0);
