#pragma once

#include "shared/entities/MovableEntity.h"

struct vec;

class SkeletalEntity : public MovableEntity {
    ENTITY_FACTORY_IMPL(SkeletalEntity)
public:
    //
    // Constructors/Destructor.
    //
    SkeletalEntity();

    //
    // Base/Core entity functions.
    //
    void preload();
    void think();

    //
    // Entity functions.
    //
    void reset();
    void respawn();

    //
    // onEvent functions.
    //
    bool onTrigger(const Entity *otherEnt, const vec &dir);
    bool onTouch(const Entity *otherEnt, const vec &dir);

    //
    // Entity member variables.
    //
//    DONTSERIALIZE MovableEntity *camera;

private:

};

