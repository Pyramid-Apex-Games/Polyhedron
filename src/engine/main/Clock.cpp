#include "Clock.h"
#include "FPS.h"
#include "shared/types.h"
#include "shared/command.h"
#include <SDL_timer.h>
#include "engine/server.h"

int curtime;                     // current frame time
int lastmillis;                  // last time
int elapsedtime;                 // elapsed frame time
int totalmillis;                 // total elapsed time
int gamespeed, paused;
int clockrealbase;
int clockvirtbase;
//int clockfix;
//int clockerror;

namespace game {
    extern int scaletime(int t);
    extern bool ispaused();
}

void clockreset()
{
    clockrealbase = SDL_GetTicks();
    clockvirtbase = totalmillis;
}

VARFP(clockerror, 990000, 1000000, 1010000, clockreset());
VARFP(clockfix, 0, 0, 1, clockreset());

int getclockmillis()
{
    int millis = SDL_GetTicks() - clockrealbase;
    if(clockfix) millis = int(millis*(double(clockerror)/1000000));
    millis += clockvirtbase;
    return std::max(millis, totalmillis);
}

void ClockFrame()
{
    static int frames = 0;
    int millis = getclockmillis();
    limitfps(millis, totalmillis);
    elapsedtime = millis - totalmillis;

    static int timeerr = 0;
    int scaledtime = game::scaletime(elapsedtime) + timeerr;
    curtime = scaledtime/100;
    timeerr = scaledtime%100;
    if(/*!multiplayer(false) &&*/ curtime>200) curtime = 200;
    if(game::ispaused())
    {
        curtime = 0;
    }
    lastmillis += curtime;
    totalmillis = millis;
    updatetime();
}