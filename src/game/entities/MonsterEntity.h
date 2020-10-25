#pragma once
#include "game/entities/SkeletalEntity.h"

class MonsterEntity : public SkeletalEntity {
    ENTITY_FACTORY_IMPL(MonsterEntity);
public:
    MonsterEntity();

    void preload();
    void think();

    void render(game::RenderPass pass);
private:

};


