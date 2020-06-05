#pragma once
#include "shared/entities/Entity.h"

class EnvMapEntity : public Entity
{
    ENTITY_FACTORY_IMPL(EnvMapEntity);
public:

    int size = 0;
    int blur = 0;
};


