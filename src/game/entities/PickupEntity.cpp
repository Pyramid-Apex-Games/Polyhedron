#include "PickupEntity.h"
#include "ModelEntity.h"
#include "../engine/engine.h"
#include "../game.h"
#include "ents.h"
#include "shared/entities/EntityFactory.h"


PickupEntity::PickupEntity()
    : ModelEntity("handgun/pistol")
{
    SetAttribute("name", "PickupEntity");

    state = CS_ALIVE;
    collidetype = COLLIDE_OBB;
    physstate = PHYS_FLOOR;

    flags |= EntityFlags::EF_ANIM;
    flags |= EntityFlags::EF_SHADOWMESH;
    flags |= EntityFlags::EF_RENDER;
    flags |= EntityFlags::EF_SPAWNED;
}

void PickupEntity::preload()
{
	ModelEntity::preload();
}

void PickupEntity::think()
{

}

void PickupEntity::onAnimate(int &anim, int &basetime)
{
    anim = ANIM_MAPMODEL | ANIM_ALL | ANIM_REVERSE;
}

bool PickupEntity::onTrigger(const Entity *otherEnt, const vec &dir)
{
    if (otherEnt == nullptr) {
        return false;
    }
    conoutf("%s %s %s %f %f %f", "PickupEntity triggered by entity: ", otherEnt->GetInstanceName().c_str(),
            "from Vector Direction: ", dir.x, dir.y, dir.z);
    return true;
}

bool PickupEntity::onTouch(const Entity *otherEnt, const vec &dir)
{
    if (otherEnt == nullptr) {
        return false;
    }
    conoutf("%s %s %s %f %f %f", "PickupEntity touched by entity: ", otherEnt->GetInstanceName().c_str(),
            "from Vector Direction: ", dir.x, dir.y, dir.z);
    return true;
}


void PickupEntity::On(const Event& event)
{
}


ADD_ENTITY_TO_FACTORY_SERIALIZED(PickupEntity, "pickup", ModelEntity);
