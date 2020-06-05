#pragma once
#include "Entity.h"

class DynamicEntity : public Entity
{
    ENTITY_FACTORY_IMPL(DynamicEntity);
public:

//    void resetinterp();

    vec feetpos(float offset) const;
    vec headpos(float offset) const;

    bool crouched() const;

    float maxspeed = 25.0f;
    float radius = 2.4;
    float eyeheight = 7.0f;
    float maxheight = 8.0f;
    float aboveeye = 2.0f;
    float xradius = 1.67;
    float yradius = 1.67;
    float zmargin = 0;

    uchar state = CS_ALIVE;
    uchar editstate = CS_ALIVE;
    uchar collidetype = COLLIDE_ELLIPSE;

    bool blocked = false;
};

