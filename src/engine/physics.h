#pragma once
#include "shared/geom/vec.h"


class MovableEntity;

void modifyorient(float yaw, float pitch);
void mousemove(int dx, int dy);
bool overlapsdynent(const vec &o, float radius);
void rotatebb(vec &center, vec &radius, int yaw, int pitch, int roll = 0);
float shadowray(const vec &o, const vec &ray, float radius, int mode, MovableEntity *t = NULL);