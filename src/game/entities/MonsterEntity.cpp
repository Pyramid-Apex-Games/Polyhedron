#include "MonsterEntity.h"

MonsterEntity::MonsterEntity() {
//    ent_type = ENT_AI;
}

void MonsterEntity::preload() {
    conoutf("%s", "Preloading basemonster entity");
}

void MonsterEntity::think() {
    //moveplayer(this, 10, true);
}

void MonsterEntity::render(RenderPass pass) {
    // TODO: Fix this.
    SkeletalEntity::render(pass);
    //if(isthirdperson()) renderclient(player1, "ogro", NULL, 0, ANIM_ATTACK1, 300, player1->lastaction, player1->lastpain);
}


void MonsterEntity::On(const Event& event)
{
}


ADD_ENTITY_TO_FACTORY_SERIALIZED(MonsterEntity, "monster", SkeletalEntity);
