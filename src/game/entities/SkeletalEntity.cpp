#include "SkeletalEntity.h"
#include "cube.h"
#include "game.h"

SkeletalEntity::SkeletalEntity() : ModelEntity("actors/bones") {
	state = CS_ALIVE;
	collidetype = COLLIDE_OBB;
	physstate = PHYS_FALL;

	conoutf("%s", "Preloading player entity");
	setAttribute("name", "SkeletalEntity");
}

//SkeletalEntity::~SkeletalEntity() {
	//delete camera;
//}

void SkeletalEntity::think()
{
	moveplayer(this, 10, true);
}

namespace
{
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

    static std::array<std::string, NUMANIMS> AnimationNameMapping {
        "mapmodel",
        "dead", "dying",
        "idle", "run N", "run NE", "run E", "run SE", "run S", "run SW", "run W", "run NW",
        "jump", "jump N", "jump NE", "jump E", "jump SE", "jump S", "jump SW", "jump W", "jump NW",
        "sink", "swim",
        "crouch", "crouch N", "crouch NE", "crouch E", "crouch SE", "crouch S", "crouch SW", "crouch W", "crouch NW",
        "crouch jump", "crouch jump N", "crouch jump NE", "crouch jump E", "crouch jump SE", "crouch jump S", "crouch jump SW", "crouch jump W", "crouch jump NW",
        "crouch sink", "crouch swim",
        "shoot", "melee",
        "pain",
        "edit", "lag", "taunt", "win", "lose",
        "gun idle", "gun shoot", "gun melee",
        "vwep idle", "vwep shoot", "vwep melee"
    };

    bool PatternMatch(const char *name, const char *pattern)
    {
        for(;; pattern++)
        {
            const char *s = name;
            char c;
            for(;; pattern++)
            {
                c = *pattern;
                if(!c || c=='|') break;
                else if(c=='*')
                {
                    if(!*s || iscubespace(*s)) break;
                    do s++; while(*s && !iscubespace(*s));
                }
                else if(c!=*s) break;
                else s++;
            }
            if(!*s && (!c || c=='|')) return true;
            pattern = strchr(pattern, '|');
            if(!pattern) break;
        }
        return false;
    }
}

int SkeletalEntity::AnimationsCount()
{
    return AnimationNameMapping.size();
}

void SkeletalEntity::FindAnimations(const std::string& pattern, vector<int> &anims)
{
    for(int i = 0; i < AnimationNameMapping.size(); ++i)
    {
        if (PatternMatch(AnimationNameMapping[i].c_str(), pattern.c_str()))
        {
            anims.add(i);
        }
    }
}

//extern void rendermodel(const char *mdl, int anim, const vec &o, float yaw, float pitch, float roll, int flags, MovableEntity *d, modelattach *a, int basetime, int basetime2, float size, const vec4 &color);
void SkeletalEntity::render(game::RenderPass pass)
{
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
}

void SkeletalEntity::on(const Event& event)
{
	switch(event.type)
	{
	    case EntityEventType::Spawn:
        {
            if (editmode)
            {
                state = CS_EDITING;
            }
            else
            {
                state = CS_ALIVE;
            }


        } break;
	    case EntityEventType::ClearSpawn:
        {
            state = CS_SPECTATOR;


        } break;
	}
}


ADD_ENTITY_TO_FACTORY_SERIALIZED(SkeletalEntity, "skeletal", ModelEntity)
