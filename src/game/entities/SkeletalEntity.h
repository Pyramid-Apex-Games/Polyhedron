#pragma once

#include "game/entities/ModelEntity.h"
#include "shared/tools/vector.h"
#include <string>

struct vec;

class SkeletalEntity : public ModelEntity {
    ENTITY_FACTORY_IMPL(SkeletalEntity)
public:
    SkeletalEntity();

    void think();

    void reset();
    void respawn();

    bool onTrigger(const Entity *otherEnt, const vec &dir);
    bool onTouch(const Entity *otherEnt, const vec &dir);

    static int AnimationsCount();
    static void FindAnimations(const std::string& pattern, vector<int> &anims);
private:

};

