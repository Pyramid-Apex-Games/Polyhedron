#include "../game.h"
// #include "baseentity.h"
#include "LightEntity.h"
#include "shared/entities/EntityFactory.h"

LightEntity::LightEntity() : Entity() {

//    ent_type = ENT_INANIMATE;
//    et_type = ET_GAMESPECIFIC;
//    game_type = GAMEENTITY;

//    setAttribute("name", "DynamicLight");
}


void LightEntity::preload() {
    conoutf("%s", "Preloading dynamiclight entity");
}

void LightEntity::think() {

}

void LightEntity::render(game::RenderPass pass) {
}

// TODO: Add other optional arguments, so all can be done in 1 command. Kindly using other method functions such as fade time or flicker style, or even interval speeds.
void LightEntity::setState(DYNAMIC_LIGHT_STATE &_lightState) {
    // Change the state.
    lightState = _lightState;
}


void LightEntity::on(const Event& event)
{
}

ADD_ENTITY_TO_FACTORY_SERIALIZED(LightEntity, "light", Entity);
