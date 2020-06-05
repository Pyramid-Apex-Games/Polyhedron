#pragma once
#include "shared/entities/Entity.h"

class SoundEntity : public Entity
{
    ENTITY_FACTORY_IMPL(SoundEntity);
public:

    int soundIndex;
};


