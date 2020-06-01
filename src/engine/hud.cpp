#include "engine/hud.h"
#include "engine/texture.h"

matrix4 hudmatrix, hudmatrixstack[64];
int hudmatrixpos = 0;

void resethudmatrix()
{
    hudmatrixpos = 0;
    GLOBALPARAM(hudmatrix, hudmatrix);
}

void pushhudmatrix()
{
    if(hudmatrixpos >= 0 && hudmatrixpos < int(sizeof(hudmatrixstack)/sizeof(hudmatrixstack[0]))) hudmatrixstack[hudmatrixpos] = hudmatrix;
    ++hudmatrixpos;
}

void flushhudmatrix(bool flushparams)
{
    GLOBALPARAM(hudmatrix, hudmatrix);
    if(flushparams && Shader::lastshader) Shader::lastshader->flushparams();
}

void pophudmatrix(bool flush, bool flushparams)
{
    --hudmatrixpos;
    if(hudmatrixpos >= 0 && hudmatrixpos < int(sizeof(hudmatrixstack)/sizeof(hudmatrixstack[0])))
    {
        hudmatrix = hudmatrixstack[hudmatrixpos];
        if(flush) flushhudmatrix(flushparams);
    }
}

void pushhudscale(float sx, float sy)
{
    if(!sy) sy = sx;
    pushhudmatrix();
    hudmatrix.scale(sx, sy, 1);
    flushhudmatrix();
}

void pushhudtranslate(float tx, float ty, float sx, float sy)
{
    if(!sy) sy = sx;
    pushhudmatrix();
    hudmatrix.translate(tx, ty, 0);
    if(sy) hudmatrix.scale(sx, sy, 1);
    flushhudmatrix();
}
