#pragma once

struct Texture;
struct materialsurface;

extern int vertwater, waterreflect, caustics;
extern float watersx1, watersy1, watersx2, watersy2;

#define GETMATIDXVAR(name, var, type) \
    type get##name##var(int mat) \
    { \
        switch(mat&MATF_INDEX) \
        { \
            default: case 0: return name##var; \
            case 1: return name##2##var; \
            case 2: return name##3##var; \
            case 3: return name##4##var; \
        } \
    }

const bvec &getwatercolour(int mat);
const bvec &getwaterdeepcolour(int mat);
const bvec &getwaterdeepfade(int mat);
const bvec &getwaterrefractcolour(int mat);
const bvec &getwaterfallcolour(int mat);
const bvec &getwaterfallrefractcolour(int mat);
int getwaterfog(int mat);
int getwaterdeep(int mat);
int getwaterspec(int mat);
float getwaterrefract(int mat);
int getwaterfallspec(int mat);
float getwaterfallrefract(int mat);

const bvec &getlavacolour(int mat);
int getlavafog(int mat);
float getlavaglowmin(int mat);
float getlavaglowmax(int mat);
int getlavaspec(int mat);

const bvec &getglasscolour(int mat);
float getglassrefract(int mat);
int getglassspec(int mat);

void renderwater();
void renderwaterfalls();
void renderlava();
void renderlava(const materialsurface &m, Texture *tex, float scale);
void loadcaustics(bool force = false);
void renderwaterfog(int mat, float blend);
void preloadwatershaders(bool force = false);