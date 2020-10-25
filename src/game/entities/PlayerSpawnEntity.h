#pragma once
#include "shared/entities/Entity.h"


class PlayerSpawnEntity : public Entity {
    ENTITY_FACTORY_IMPL(PlayerSpawnEntity);
public:
    PlayerSpawnEntity();

    void preload();
    void think();

    void reset();

private:
};

