#include "font.h"
#include "scriptexport.h"
#include "texture.h"
#include "tools/cubestr.h"
#include "shared/cube.h"
#include <algorithm>

namespace {
    static hashnameset<font> fonts;
    static font *fontdef = NULL;
    static int fontdeftex = 0;
    static vector<font *> fontstack;

    font *curfont = NULL;
    int curfonttex = 0;
}

font::font() : name(NULL) {}
font::~font() { DELETEA(name); }

font* font::CurrentFont()
{
    return curfont;
}


void setfont(font *f)
{
    if (f)
    {
        curfont = f;
    }
}

SCRIPTEXPORT_AS(font) void newfont(char *name, char *tex, int *defaultw, int *defaulth, int *scale)
{
    font *f = &fonts[name];
    if(!f->name) f->name = newcubestr(name);
    f->texs.clear();
    f->texs.emplace_back(textureload(tex));
    f->chars.clear();
    f->charoffset = '!';
    f->defaultw = *defaultw;
    f->defaulth = *defaulth;
    f->scale = *scale > 0 ? *scale : f->defaulth;
    f->bordermin = 0.49f;
    f->bordermax = 0.5f;
    f->outlinemin = -1;
    f->outlinemax = 0;

    fontdef = f;
    fontdeftex = 0;
}

SCRIPTEXPORT void fontborder(float *bordermin, float *bordermax)
{
    if(!fontdef) return;

    fontdef->bordermin = *bordermin;
    fontdef->bordermax = std::max(*bordermax, *bordermin+0.01f);
}

SCRIPTEXPORT void fontoutline(float *outlinemin, float *outlinemax)
{
    if(!fontdef) return;

    fontdef->outlinemin = std::min(*outlinemin, *outlinemax-0.01f);
    fontdef->outlinemax = *outlinemax;
}

SCRIPTEXPORT void fontoffset(char *c)
{
    if(!fontdef) return;

    fontdef->charoffset = c[0];
}

SCRIPTEXPORT void fontscale(int *scale)
{
    if(!fontdef) return;

    fontdef->scale = *scale > 0 ? *scale : fontdef->defaulth;
}

SCRIPTEXPORT void fonttex(char *s)
{
    if(!fontdef) return;

    Texture *t = textureload(s);
    loopv(fontdef->texs) if(fontdef->texs[i] == t) { fontdeftex = i; return; }
    fontdeftex = fontdef->texs.size();
    fontdef->texs.emplace_back(t);
}

SCRIPTEXPORT void fontchar(float *x, float *y, float *w, float *h, float *offsetx, float *offsety, float *advance)
{
    if(!fontdef) return;

    font::charinfo &c = fontdef->chars.emplace_back();
    c.x = *x;
    c.y = *y;
    c.w = *w ? *w : fontdef->defaultw;
    c.h = *h ? *h : fontdef->defaulth;
    c.offsetx = *offsetx;
    c.offsety = *offsety;
    c.advance = *advance ? *advance : c.offsetx + c.w;
    c.tex = fontdeftex;
}

SCRIPTEXPORT void fontskip(int *n)
{
    if(!fontdef) return;
    loopi(std::max(*n, 1))
    {
        font::charinfo &c = fontdef->chars.emplace_back();
        c.x = c.y = c.w = c.h = c.offsetx = c.offsety = c.advance = 0;
        c.tex = 0;
    }
}

SCRIPTEXPORT void fontalias(const char *dst, const char *src)
{
    font *s = fonts.access(src);
    if(!s) return;
    font *d = &fonts[dst];
    if(!d->name) d->name = newcubestr(dst);
    d->texs = s->texs;
    d->chars = s->chars;
    d->charoffset = s->charoffset;
    d->defaultw = s->defaultw;
    d->defaulth = s->defaulth;
    d->scale = s->scale;
    d->bordermin = s->bordermin;
    d->bordermax = s->bordermax;
    d->outlinemin = s->outlinemin;
    d->outlinemax = s->outlinemax;

    fontdef = d;
    fontdeftex = d->texs.size()-1;
}

font *findfont(const char *name)
{
    return fonts.access(name);
}

bool setfont(const char *name)
{
    font *f = fonts.access(name);
    if(!f) return false;
    curfont = f;
    return true;
}


void pushfont()
{
    fontstack.emplace_back(curfont);
}

bool popfont()
{
    if(fontstack.empty()) return false;
    curfont = fontstack.pop_back();
    return true;
}

void gettextres(int &w, int &h)
{
    if(w < MINRESW || h < MINRESH)
    {
        if(MINRESW > w*MINRESH/h)
        {
            h = h*MINRESW/w;
            w = MINRESW;
        }
        else
        {
            w = w*MINRESH/h;
            h = MINRESH;
        }
    }
}

float text_widthf(const char *str)
{
    float width, height;
    text_boundsf(str, width, height);
    return width;
}

SCRIPTEXPORT void tabify(const char *str, int *numtabs)
{
    int tw = std::max(*numtabs, 0)*FONTTAB-1, tabs = 0;
    for(float w = text_widthf(str); w <= tw; w = TEXTTAB(w)) ++tabs;
    int len = strlen(str);
    char *tstr = newcubestr(len + tabs);
    memcpy(tstr, str, len);
    memset(&tstr[len], '\t', tabs);
    tstr[len+tabs] = '\0';
    cubestrret(tstr);
}

void reloadfonts()
{
    enumerate(fonts, font, f,
              loopv(f.texs) if(!reloadtexture(*f.texs[i])) fatal("failed to reload font texture");
    );
}