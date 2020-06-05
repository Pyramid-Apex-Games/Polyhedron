#include "SkeletalEntity.h"
#include "cube.h"
#include "game.h"

SkeletalEntity::SkeletalEntity() : MovableEntity() {
	state = CS_ALIVE;
//	et_type = ET_GAMESPECIFIC;
//	ent_type = ENT_PLAYER;
//	game_type = PLAYER;
	collidetype = COLLIDE_OBB;
	physstate = PHYS_FALL;
	// Load in our player entity model.
	conoutf("%s", "Preloading player entity");
	preloadmodel("player/male");
	// Reset.

	setAttribute("name", "SkeletalEntity");
//preloadmodel("player/female");
	// Camera.
//	camera = new MovableEntity();
}

//SkeletalEntity::~SkeletalEntity() {
	//delete camera;
//}

void SkeletalEntity::preload() {
	// Load in our player entity model.
	conoutf("%s", "Preloading player entity");
	preloadmodel("player/male");

	state = CS_ALIVE;
//	et_type = ET_GAMESPECIFIC;
//	ent_type = ENT_PLAYER;
//	game_type = PLAYER;
	collidetype = COLLIDE_OBB;
	physstate = PHYS_FALL;
}

void SkeletalEntity::think() {
	moveplayer(this, 10, true);
}

enum
{
	ANIM_DEAD = ANIM_GAMESPECIFIC, ANIM_DYING,
	ANIM_IDLE, ANIM_RUN_N, ANIM_RUN_NE, ANIM_RUN_E, ANIM_RUN_SE, ANIM_RUN_S, ANIM_RUN_SW, ANIM_RUN_W, ANIM_RUN_NW,
	ANIM_JUMP, ANIM_JUMP_N, ANIM_JUMP_NE, ANIM_JUMP_E, ANIM_JUMP_SE, ANIM_JUMP_S, ANIM_JUMP_SW, ANIM_JUMP_W, ANIM_JUMP_NW,
	ANIM_SINK, ANIM_SWIM,
	ANIM_CROUCH, ANIM_CROUCH_N, ANIM_CROUCH_NE, ANIM_CROUCH_E, ANIM_CROUCH_SE, ANIM_CROUCH_S, ANIM_CROUCH_SW, ANIM_CROUCH_W, ANIM_CROUCH_NW,
	ANIM_CROUCH_JUMP, ANIM_CROUCH_JUMP_N, ANIM_CROUCH_JUMP_NE, ANIM_CROUCH_JUMP_E, ANIM_CROUCH_JUMP_SE, ANIM_CROUCH_JUMP_S, ANIM_CROUCH_JUMP_SW, ANIM_CROUCH_JUMP_W, ANIM_CROUCH_JUMP_NW,
	ANIM_CROUCH_SINK, ANIM_CROUCH_SWIM,
	ANIM_SHOOT, ANIM_MELEE,
	ANIM_PAIN,
	ANIM_EDIT, ANIM_LAG, ANIM_TAUNT, ANIM_WIN, ANIM_LOSE,
	ANIM_GUN_IDLE, ANIM_GUN_SHOOT, ANIM_GUN_MELEE,
	ANIM_VWEP_IDLE, ANIM_VWEP_SHOOT, ANIM_VWEP_MELEE,
	NUMANIMS
};
//extern void rendermodel(const char *mdl, int anim, const vec &o, float yaw, float pitch, float roll, int flags, MovableEntity *d, modelattach *a, int basetime, int basetime2, float size, const vec4 &color);
void SkeletalEntity::render(game::RenderPass pass)
{
	if (pass == game::RenderPass::Main)
	{
		if (isthirdperson() && state != CS_EDITING)
		{
			// Calculate the position.
			vec pos = o;
			pos.z -= eyeheight;
			rendermodel("actors/player/male", ANIM_JUMP, pos, d.x, d.y, 0, MDL_CULL_VFC | MDL_CULL_DIST | MDL_CULL_OCCLUDED);
		}
	}
}

bool SkeletalEntity::onTrigger(const Entity *otherEnt, const vec &dir) {
    if (otherEnt != nullptr) {
        conoutf("%s '%s' %s %s %s %f %f %f", "SkeletalEntity: ", name.c_str(), " triggered by entity: ", otherEnt->classname.c_str(),
            "from Vector Direction: ", dir.x, dir.y, dir.z);
            return true;
    } else {
        return false;
    }
}

bool SkeletalEntity::onTouch(const Entity *otherEnt, const vec &dir) {
     if (otherEnt != nullptr) {
        conoutf("%s %s %s %f %f %f", "SkeletalEntity touched by entity: ", otherEnt->classname.c_str(),
            "from Vector Direction: ", dir.x, dir.y, dir.z);
        return true;
    } else {
        return false;
    }
}

void SkeletalEntity::reset()
{
	on(EntityEventClearSpawn());
}

void SkeletalEntity::respawn()
{
	on(EntityEventSpawn());

	if (editmode)
	{
		state = CS_EDITING;
	}
	else
	{
		state = CS_ALIVE;
	}
}



void SkeletalEntity::on(const Event& event)
{
}
// Link entity class to the factory.


ADD_ENTITY_TO_FACTORY_SERIALIZED(SkeletalEntity, "player", MovableEntity)
