#ifndef __GAME_H__
#define __GAME_H__

#include "cube.h"
#include "ents.h"


class SkeletalEntity;
class Entity;

namespace game
{
    // Extern variables.
    extern SkeletalEntity *player1;  // Main player entity in the game code.
    extern int maptime, maprealtime;            // Times.
    extern cubestr clientmap;                   // The map the client is currently running or loading.

    // Entities.
    extern void clearworld();

    // Update functions.
    extern void updateentities();

    // Render functions.
    extern void rendergame(RenderPass pass);
    extern void renderobjects();

    // HUD functions.
    extern float clipconsole(float w, float h);

    // Physics.
    extern void physicstrigger(MovableEntity *d, bool local, int floorlevel, int waterlevel, int material);

    // Renderer.
    #define MAXTEAMS 2
    struct playermodelinfo
    {
        const char *model[1+MAXTEAMS], *hudguns[1+MAXTEAMS],
                   *icon[1+MAXTEAMS];
        bool ragdoll;
    };

    extern void saveragdoll(Entity *d);
    extern void clearragdolls();
    extern void moveragdolls();
    extern const playermodelinfo &getplayermodelinfo(Entity *d);
    extern int getplayercolor(Entity *d, int team);
    extern int chooserandomplayermodel(int seed);
    extern void syncplayer();
    extern void swayhudgun(int curtime);
    extern vec hudgunorigin(int gun, const vec &from, const vec &to, Entity *d);
}

#endif

