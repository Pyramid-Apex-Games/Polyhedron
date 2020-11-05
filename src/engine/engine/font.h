#pragma once
#include "tools/vector_util.h"

struct Texture;
struct Shader;
struct matrix4x3;

// rendertext
struct font
{
    struct charinfo
    {
        float x, y, w, h, offsetx, offsety, advance;
        int tex;
    };

    char *name;
    vector<Texture *> texs;
    vector<charinfo> chars;
    int charoffset, defaultw, defaulth, scale;
    float bordermin, bordermax, outlinemin, outlinemax;

    font();
    ~font();
    static font* CurrentFont();
};

#define FONTH (font::CurrentFont()->scale)
#define FONTW (FONTH/2)
#define MINRESW 640
#define MINRESH 480

extern Shader *textshader;
extern const matrix4x3 *textmatrix;
extern float textscale;

extern font *findfont(const char *name);
extern void reloadfonts();

void setfont(font *f);
bool setfont(const char* name);
void newfont(char *name, char *tex, int *defaultw, int *defaulth, int *scale);
void fontborder(float *bordermin, float *bordermax);
void fontoutline(float *outlinemin, float *outlinemax);
void fontoffset(char *c);
void fontscale(int *scale);
void fonttex(char *s);
void fontchar(float *x, float *y, float *w, float *h, float *offsetx, float *offsety, float *advance);
void fontskip(int *n);
void fontalias(const char *dst, const char *src);
font *findfont(const char *name);
bool setfont(const char *name);
void pushfont();
bool popfont();
void gettextres(int &w, int &h);
float text_widthf(const char *str);
void tabify(const char *str, int *numtabs);
void reloadfonts();

#define FONTTAB (4*FONTW)
#define TEXTTAB(x) ((int((x)/FONTTAB)+1.0f)*FONTTAB)