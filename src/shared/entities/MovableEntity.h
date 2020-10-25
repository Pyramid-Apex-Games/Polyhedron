#pragma once
#include "animinfo.h"
#include "DynamicEntity.h"

struct occludequery;
struct ragdolldata;

class MovableEntity : public DynamicEntity
{
    ENTITY_FACTORY_IMPL(MovableEntity)
public:
    void stopmoving();
    vec abovehead();

    void resetinterp();

    DONTSERIALIZE bool k_left = false;
    DONTSERIALIZE bool k_right = false;
    DONTSERIALIZE bool k_up = false;
    DONTSERIALIZE bool k_down = false;
    DONTSERIALIZE int inwater = 0;
    DONTSERIALIZE int timeinair = 0;
    DONTSERIALIZE bool jumping = false;
    DONTSERIALIZE char strafe = 0;
    DONTSERIALIZE char move = 0;
    DONTSERIALIZE char crouching = 0;
    DONTSERIALIZE uchar physstate = PHYS_FLOOR;
    DONTSERIALIZE vec vel = vec(0, 0, 0);
    DONTSERIALIZE vec falling = vec(0, 0, 0);
    DONTSERIALIZE vec floor = vec(0, 0, 1);
    DONTSERIALIZE animinterpinfo animinterp[MAXANIMPARTS] { 0 };
    DONTSERIALIZE ragdolldata *ragdoll = nullptr;
    DONTSERIALIZE occludequery *query = nullptr;
    DONTSERIALIZE int lastrendered = -1;

    DONTSERIALIZE vec newpos = vec(0, 0, 0);
    DONTSERIALIZE vec deltapos = vec(0, 0, 0);
protected:
};
