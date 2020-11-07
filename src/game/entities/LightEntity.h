#pragma once
#include "shared/entities/Entity.h"

class LightEntity : public Entity {
    ENTITY_FACTORY_IMPL(LightEntity);
public:
    LightEntity();

    void preload();
    void think();

    enum LIGHT_STATE {
        ON,             // Light is on.
        OFF,            // Light is off.
    };
    enum LIGHT_STYLE {
        DEFAULT,        // Non Flickering Light
        SINE,           // Sine Wave animating Light
    };

    void setState(LightEntity::LIGHT_STATE &state);
    void setStyle(LightEntity::LIGHT_STYLE &style);

public:
    LightEntity::LIGHT_STATE lightState;
    LightEntity::LIGHT_STYLE lightStyle;
    vec lightColor;
    float radius;
    int type;
};
