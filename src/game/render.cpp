#include "game.h"
#include "engine/scriptexport.h"
#include "entities.h"
#include "entities/SkeletalEntity.h"

namespace game
{
    void renderentities(RenderPass pass)
    {
		auto& ents = getents();
		
        for (int i = 0; ents.inrange(i); ++i)
        {
			auto& entity = ents[i];
			
			entity->renderImpl(pass);
        }
    }

    VARP(hudgun, 0, 1, 1);
    VARP(hudgunsway, 0, 1, 1);

    FVAR(swaystep, 1, 35.0f, 100);
    FVAR(swayside, 0, 0.10f, 1);
    FVAR(swayup, -1, 0.15f, 1);

    float swayfade = 0, swayspeed = 0, swaydist = 0;
    vec swaydir(0, 0, 0);

    void swayhudgun(int curtime) {

    }


    void drawhudmodel(Entity *d, int anim, int basetime) {

    }

    void drawhudgun() {

    }

    void renderplayerpreview(int model, int color, int team, int weap) {

    }

    vec hudgunorigin(int gun, const vec &from, const vec &to, Entity *d) {
        vec offset(from);

        return offset;
    }

    int numanims() {
        return 0;
    }

    __attribute__((used)) void findanims(const char *pattern, vector<int> &anims)
    {
//        loopi(sizeof(animnames)/sizeof(animnames[0])) if(matchanim(animnames[i], pattern)) anims.add(i);
    }

    void preloadweapons() {

    }

    void preloadsounds()
    {

    }
}

SCRIPTEXPORT int getplayercolor(int team, int color)
{
    switch(team)
    {
        case 1: return 0x0000FF;
        case 2: return 0xFF0000;
        default: return 0xFFFF77;
    }
}


// >>>>>>>>>> SCRIPTBIND >>>>>>>>>>>>>> //
#if 0
#include "/Users/micha/dev/ScMaMike/src/build/binding/..+game+render.binding.cpp"
#endif
// <<<<<<<<<< SCRIPTBIND <<<<<<<<<<<<<< //
