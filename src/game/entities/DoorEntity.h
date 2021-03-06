#pragma once
#include "ModelEntity.h"

class DoorEntity : public ModelEntity {
    ENTITY_FACTORY_IMPL(DoorEntity);
public:
    DoorEntity();

    void preload();
    void think();

    bool onTrigger(const Entity *otherEnt, const vec &dir);
    bool onTouch(const Entity *otherEnt, const vec &dir);

    void onAnimate(int &anim, int &basetime);

private:
    vec pivot = vec(0.0f, 0.0f, 0.0f);
    vec axis = vec(0.0f, 1.0f, 0.0f);
};
