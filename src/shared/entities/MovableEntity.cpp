#include "MovableEntity.h"
#include "animinfo.h"

namespace {
	float aboveeye = 9.0f;
}

void MovableEntity::resetinterp() {
    newpos = o;
    deltapos = vec(0, 0, 0);
}

// Stop movement.
void MovableEntity::stopmoving()
{
    k_left = k_right = k_up = k_down = jumping = false;
    move = strafe = crouching = 0;
}


vec MovableEntity::abovehead() {
    return vec(o).addz(aboveeye+4);
}


void MovableEntity::On(const Event& event)
{
}

ADD_ENTITY_TO_FACTORY_SERIALIZED(MovableEntity, "movable", DynamicEntity);
