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

    static const std::array<std::string, NUMANIMS> AnimationNameMapping {
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

SkeletalEntity::AnimationParams SkeletalEntity::CalculateAnimation() const
{
    int anim = ANIM_IDLE|ANIM_LOOP;
    int attack = 0;
    int delay = 0;

//        if(lastattack >= 0)
//        {
//            attack = attacks[lastattack].anim;
//            delay = attacks[lastattack].attackdelay+50;
//        }
//        if(intermission && state!=CS_DEAD)
//        {
//            anim = attack = ANIM_LOSE|ANIM_LOOP;
//            if(validteam(team) ? bestteams.htfind(team)>=0 : bestplayers.find(d)>=0) anim = attack = ANIM_WIN|ANIM_LOOP;
//        }
//        /*else*/ if(state==CS_ALIVE && lasttaunt && lastmillis-lasttaunt<1000 && lastmillis-lastaction>delay)
//        {
//            lastaction = lasttaunt;
//            anim = attack = ANIM_TAUNT;
//            delay = 1000;
//        }
//        modelattach a[5];
//        int ai = 0;
//        if(guns[gunselect].vwep)
//        {
//            int vanim = ANIM_VWEP_IDLE|ANIM_LOOP, vtime = 0;
//            if(lastaction && lastattack >= 0 && attacks[lastattack].gun==gunselect && lastmillis < lastaction + delay)
//            {
//                vanim = attacks[lastattack].vwepanim;
//                vtime = lastaction;
//            }
//            a[ai++] = modelattach("tag_weapon", guns[gunselect].vwep, vanim, vtime);
//        }
//        if(mainpass && !(flags&MDL_ONLYSHADOW))
//        {
//            muzzle = vec(-1, -1, -1);
//            if(guns[gunselect].vwep) a[ai++] = modelattach("tag_muzzle", &muzzle);
//        }
//        const char *mdlname = mdl.model[validteam(team) ? team : 0];
//        float yaw = testanims && d==player1 ? 0 : yaw,
//              pitch = testpitch && d==player1 ? testpitch : pitch;
//        vec o = feetpos();
    int basetime = 0;
//        if(animoverride) anim = (animoverride<0 ? ANIM_ALL : animoverride)|ANIM_LOOP;
    if(state==CS_DEAD)
    {
        anim = ANIM_DYING|ANIM_NOPITCH;
//            basetime = lastpain;
        if(ragdoll/* && mdl.ragdoll*/) anim |= ANIM_RAGDOLL;
        else if(lastmillis-basetime>1000) anim = ANIM_DEAD|ANIM_LOOP|ANIM_NOPITCH;
    }
    else if(state==CS_EDITING || state==CS_SPECTATOR) anim = ANIM_EDIT|ANIM_LOOP;
    else if(state==CS_LAGGED)                            anim = ANIM_LAG|ANIM_LOOP;

    {
//            if(lastmillis-lastpain < 300)
//            {
//                anim = ANIM_PAIN;
//                basetime = lastpain;
//            }
//            else if(lastpain < lastaction && lastmillis-lastaction < delay)
//            {
//                anim = attack;
//                basetime = lastaction;
//            }

        if(inwater && physstate<=PHYS_FALL)
        {
            anim |= (((game::allowmove(this) && (move || strafe)) || vel.z+falling.z>0 ? ANIM_SWIM : ANIM_SINK)|ANIM_LOOP)<<ANIM_SECONDARY;
        }
        else
        {
            static const int dirs[9] =
            {
                ANIM_RUN_SE, ANIM_RUN_S, ANIM_RUN_SW,
                ANIM_RUN_E,  0,          ANIM_RUN_W,
                ANIM_RUN_NE, ANIM_RUN_N, ANIM_RUN_NW
            };
            int dir = dirs[(move+1)*3 + (strafe+1)];
            if(timeinair>100)
            {
                anim |= ((dir ? dir+ANIM_JUMP_N-ANIM_RUN_N : ANIM_JUMP) | ANIM_END) << ANIM_SECONDARY;
            }
            else if(dir && game::allowmove(this))
            {
                anim |= (dir | ANIM_LOOP) << ANIM_SECONDARY;
            }
        }

        if(crouching) switch((anim>>ANIM_SECONDARY)&ANIM_INDEX)
        {
            case ANIM_IDLE: anim &= ~(ANIM_INDEX<<ANIM_SECONDARY); anim |= ANIM_CROUCH<<ANIM_SECONDARY; break;
            case ANIM_JUMP: anim &= ~(ANIM_INDEX<<ANIM_SECONDARY); anim |= ANIM_CROUCH_JUMP<<ANIM_SECONDARY; break;
            case ANIM_SWIM: anim &= ~(ANIM_INDEX<<ANIM_SECONDARY); anim |= ANIM_CROUCH_SWIM<<ANIM_SECONDARY; break;
            case ANIM_SINK: anim &= ~(ANIM_INDEX<<ANIM_SECONDARY); anim |= ANIM_CROUCH_SINK<<ANIM_SECONDARY; break;
            case 0: anim |= (ANIM_CROUCH|ANIM_LOOP)<<ANIM_SECONDARY; break;
            case ANIM_RUN_N: case ANIM_RUN_NE: case ANIM_RUN_E: case ANIM_RUN_SE: case ANIM_RUN_S: case ANIM_RUN_SW: case ANIM_RUN_W: case ANIM_RUN_NW:
                anim += (ANIM_CROUCH_N - ANIM_RUN_N) << ANIM_SECONDARY;
                break;
            case ANIM_JUMP_N: case ANIM_JUMP_NE: case ANIM_JUMP_E: case ANIM_JUMP_SE: case ANIM_JUMP_S: case ANIM_JUMP_SW: case ANIM_JUMP_W: case ANIM_JUMP_NW:
                anim += (ANIM_CROUCH_JUMP_N - ANIM_JUMP_N) << ANIM_SECONDARY;
                break;
        }

        if((anim&ANIM_INDEX)==ANIM_IDLE && (anim>>ANIM_SECONDARY)&ANIM_INDEX) anim >>= ANIM_SECONDARY;
    }
    if(!((anim>>ANIM_SECONDARY)&ANIM_INDEX)) anim |= (ANIM_IDLE|ANIM_LOOP)<<ANIM_SECONDARY;

    return AnimationParams { anim, basetime};
}

void SkeletalEntity::render(game::RenderPass pass)
{
    //extern void rendermodel(const char *mdl, int anim, const vec &o, float yaw, float pitch, float roll, int flags, MovableEntity *d, modelattach *a, int basetime, int basetime2, float size, const vec4 &color);
//    ModelEntity::render(pass);

    if (pass == game::RenderPass::Main)
    {
        if(this==game::player1 && thirdperson)
        {
            auto [anim, basetime] = CalculateAnimation();

            if(this!=game::player1) flags |= MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY;
    //        if(type==ENT_PLAYER) flags |= MDL_FULLBRIGHT;
    //        else
                flags |= MDL_CULL_DIST;
    //        if(!mainpass) flags &= ~(MDL_FULLBRIGHT | MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY | MDL_CULL_DIST);
            float trans = state == CS_LAGGED ? 0.5f : 1.0f;
            animation = anim;
            rendermodel(modelname.c_str(), anim, o, d.x, d.y, d.z, flags, this, nullptr, basetime, 0, size, vec4(color.x, color.y, color.z, trans));
            //rendermodel(mdlname, anim, o, syaw, pitch, 0, flags, d, a[0].tag ? a : NULL, basetime, 0, fade, vec4(vec::hexcolor(color), trans));
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
