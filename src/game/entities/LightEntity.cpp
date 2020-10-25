#include "../game.h"
// #include "baseentity.h"
#include "LightEntity.h"
#include "shared/entities/EntityFactory.h"

LightEntity::LightEntity() : Entity() {
    flags = EF_RENDER | EF_NOSHADOW;
}


void LightEntity::preload() {
    conoutf("%s", "Preloading dynamiclight entity");
}

void LightEntity::think() {

}


void LightEntity::setState(LightEntity::LIGHT_STATE &_lightState) { lightState = _lightState; }
void LightEntity::setStyle(LightEntity::LIGHT_STYLE &_lightStyle) { lightStyle = _lightStyle; }

void LightEntity::on(const Event& event)
{
}

ADD_ENTITY_TO_FACTORY_SERIALIZED(LightEntity, "light", Entity);
