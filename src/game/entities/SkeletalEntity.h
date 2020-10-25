#pragma once

#include "game/entities/ModelEntity.h"
#include "shared/tools/vector.h"
#include <string>

struct vec;

class SkeletalEntity : public ModelEntity {
    ENTITY_FACTORY_IMPL(SkeletalEntity)
    struct AnimationParams
    {
        int anim;
        int basetime;
    };
public:
    SkeletalEntity();

    void think();

    void reset();
    void respawn();

    bool onTrigger(const Entity *otherEnt, const vec &dir);
    bool onTouch(const Entity *otherEnt, const vec &dir);

    virtual void render(game::RenderPass pass);

    static int AnimationsCount();
    static void FindAnimations(const std::string& pattern, vector<int> &anims);
private:

    AnimationParams CalculateAnimation() const;

};

