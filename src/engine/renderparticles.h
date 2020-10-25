#pragma once
 class Entity;

extern int particlelayers;

enum { PL_ALL = 0, PL_UNDER, PL_OVER, PL_NOLAYER };

void initparticles();
void clearparticles();
void clearparticleemitters();
void seedparticles();
void updateparticles();
void debugparticles();
void renderparticles(int layer = PL_ALL);
bool printparticles(Entity *e, char *buf, int len);
void cleanupparticles();