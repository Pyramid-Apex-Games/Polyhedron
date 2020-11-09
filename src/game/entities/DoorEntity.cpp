#include "../engine/engine.h"
#include "../game.h"
#include "ents.h"
#include "ModelEntity.h"
#include "DoorEntity.h"
#include "shared/entities/EntityFactory.h"


DoorEntity::DoorEntity()
    : ModelEntity("world/door_2")
{
    // Reset.
    SetAttribute("name", "DoorEntity");

    // Setup the door its states.
    state = CS_ALIVE;
//    et_type = ET_GAMESPECIFIC;
//    ent_type = ENT_INANIMATE;
//    game_type = GAMEENTITY;
    collidetype = COLLIDE_OBB;
    physstate = PHYS_FLOOR;

    // DoorEntitys animate, makes sense.
    flags |= EntityFlags::EF_ANIM;
    flags |= EntityFlags::EF_SHADOWMESH;
    flags |= EntityFlags::EF_RENDER;
    flags |= EntityFlags::EF_SPAWNED;
}

void DoorEntity::preload()
{
	ModelEntity::preload();
}

void DoorEntity::think()
{

}

void DoorEntity::onAnimate(int &anim, int &basetime)
{
//    conoutf("OnAnimate: %i %i", anim, basetime);
    anim = ANIM_MAPMODEL | ANIM_ALL | ANIM_REVERSE;
}

bool DoorEntity::onTrigger(const Entity *otherEnt, const vec &dir)
{
    if (otherEnt == nullptr) {
        return false;
    }
    conoutf("%s %s %s %f %f %f", "DoorEntity triggered by entity: ", otherEnt->GetInstanceName().c_str(),
            "from Vector Direction: ", dir.x, dir.y, dir.z);
    return true;
}

bool DoorEntity::onTouch(const Entity *otherEnt, const vec &dir)
{
    if (otherEnt == nullptr) {
        return false;
    }
    conoutf("%s %s %s %f %f %f", "DoorEntity touched by entity: ", otherEnt->GetInstanceName().c_str(),
            "from Vector Direction: ", dir.x, dir.y, dir.z);
    return true;
}


void DoorEntity::On(const Event& event)
{
}


ADD_ENTITY_TO_FACTORY_SERIALIZED(DoorEntity, "door", ModelEntity);
