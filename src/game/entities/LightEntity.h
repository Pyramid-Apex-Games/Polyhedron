#pragma once
#include "shared/entities/Entity.h"

class LightEntity : public Entity {
    ENTITY_FACTORY_IMPL(LightEntity);
public:
    LightEntity();

    void preload();
    void think();

    enum DYNAMIC_LIGHT_STATE {
        ON,
        OFF,
        FLICKERING,
        FADING
    };
//
//			NLOHMANN_JSON_SERIALIZE_ENUM( DYNAMIC_LIGHT_STATE, {
//				{ON, "ON"},
//				{OFF, "OFF"},
//				{FLICKERING, "FLICKERING"},
//				{FADING, "FADING"},
//			});

    void setState(LightEntity::DYNAMIC_LIGHT_STATE &state);

public:
    //
    // Light states.
    //
    // Stores the current state of the dynamic light.
    LightEntity::DYNAMIC_LIGHT_STATE lightState;

    // Obviously speaks for itself, the color.
    vec4 lightColor;
    float radius;
    int type;
};
