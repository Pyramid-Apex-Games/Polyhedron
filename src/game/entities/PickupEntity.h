#pragma once
#include "ModelEntity.h"

class PickupEntity : public ModelEntity {
    ENTITY_FACTORY_IMPL(PickupEntity);
public:
    PickupEntity();

    void preload();
    void think();

    bool onTrigger(const Entity *otherEnt, const vec &dir);
    bool onTouch(const Entity *otherEnt, const vec &dir);

    void onAnimate(int &anim, int &basetime);
};
