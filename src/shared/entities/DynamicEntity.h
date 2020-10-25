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

    float maxspeed = 60.0f;
    float radius = 3.6f;
    float eyeheight = 0.f;
    float maxheight = 25.f;
    float aboveeye = 18.f;
    float xradius = 1.67f;
    float yradius = 1.67f;
    float zmargin = 0;

    uchar state = CS_ALIVE;
    uchar editstate = CS_ALIVE;
    uchar collidetype = COLLIDE_ELLIPSE;

    bool blocked = false;
};

