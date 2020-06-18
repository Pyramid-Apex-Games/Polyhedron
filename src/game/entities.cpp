#include "engine.h"
#include "game.h"
#include "entities.h"
#include "engine/nui/nui.h"

// Base entities.
#include "entities/MonsterEntity.h"
#include "entities/ModelEntity.h"

// Game entities.
#include "entities/DoorEntity.h"
#include "entities/LightEntity.h"
#include "entities/PlayerSpawnEntity.h"
#include "entities/SkeletalEntity.h"
#include "shared/entities/EntityFactory.h"


#ifndef STANDALONE

vector<Entity *> &getents() {
    static vector<Entity *> g_ents;
    return g_ents;
}

bool mayattach(MovableEntity *e) { return false; }
bool mayattach(Entity *e) { return false; }
bool entitities_attachable(Entity *e, Entity *a) { return false; }

const char *itemname(int i)
{
    return nullptr;
#if 0
    int t = ents[i]->type;
    if(!validitem(t)) return NULL;
    return itemstats[t-I_FIRST].name;
#endif
}

int itemicon(int i)
{
    return -1;
#if 0
    int t = ents[i]->type;
    if(!validitem(t)) return -1;
    return itemstats[t-I_FIRST].icon;
#endif
}

const char *entmdlname(int type)
{
    return "";
}

const char *entmodel(const Entity *e)
{
    auto modelEnt = dynamic_cast<const ModelEntity*>(e);
    if (modelEnt)
    {
        return modelEnt->getModelName().c_str();
    }
    return "";
}

void preloadentities()
{
    // Execute preload actions for entities.
    loopv(getents())
    {
        if (getents().inrange(i) && getents()[i] != nullptr) {
            // Let's go at it!
            auto entity = getents()[i];
            send_entity_event(entity, EntityEventPrecache());
         }
    }

    // Specifically load in the client player model.
    if (game::player1 != nullptr) {
        send_entity_event(game::player1, EntityEventPrecache());
    }
}

void resetspawns()
{
    loopv(getents())
        if (getents().inrange(i))
            send_entity_event(i, EntityEventClearSpawn());

    if (game::player1 != nullptr)
        send_entity_event(game::player1, EntityEventClearSpawn());
}

void setspawn(int i, bool on)
{
    if(getents().inrange(i))
    {
        if (on)
        {
            send_entity_event(i, EntityEventSpawn());
        }
        else
        {
            send_entity_event(i, EntityEventClearSpawn());
        }
    }
}

void deletegameentity(Entity *e)
{
    //Send global event EventEntityRemovedFromMap
    delete e;
}

// Deletes all game entities in the stack.
void clearents()
{
    // Delete stack entities.
    while(getents().length()) deletegameentity(getents().pop());
}

void animatemapmodel(const Entity *e, int &anim, int &basetime)
{/*        const fpsentity &f = (const fpsentity &)e;
    if(validtrigger(f.attr3)) switch(f.triggerstate)
    {
        case TRIGGER_RESET: anim = ANIM_TRIGGER|ANIM_START; break;
        case TRIGGERING: anim = ANIM_TRIGGER; basetime = f.lasttrigger; break;
        case TRIGGERED: anim = ANIM_TRIGGER|ANIM_END; break;
        case TRIGGER_RESETTING: anim = ANIM_TRIGGER|ANIM_REVERSE; basetime = f.lasttrigger; break;
    }*/
    //const classes::BaseMapModelEntity *ent = (const classes::BaseMapModelEntity&)e;
    //anim = ANIM_MAPMODEL | ANIM_START | ANIM_LOOP;
    //basetime = SDL_GetTicks() - e.reserved;
    //e.reserved = SDL_GetTicks();

}

void entradius(Entity *e, bool color)
{
/*		switch(e->game_type)
    {
        case TELEPORT:
            loopv(getents()) if(getents()[i]->game_type == TELEDEST && e->attr1==getents()[i]->attr2)
            {
                renderentarrow(e, vec(getents()[i]->o).sub(e->o).normalize(), e->o.dist(getents()[i]->o));
                break;
            }
            break;

        case JUMPPAD:
            renderentarrow(e, vec((int)(char)e->attr3*10.0f, (int)(char)e->attr2*10.0f, e->attr1*12.5f).normalize(), 4);
            break;

        case FLAG:
        case TELEDEST:
        {
            vec dir;
            vecfromyawpitch(e->attr1, 0, 1, 0, dir);
            renderentarrow(e, dir, 4);
            break;
        }
    }*/
}

//bool printent(Entity *e, char *buf, int len)
//{
//    return false;
//}

const char *entname(int i)
{
    auto& ents = getents();

    if (i >= 0 && i < ents.length())
    {
        Entity *ent = ents[i];
        return ent->name.c_str();
    }

    return "";
}

void editent(int i, bool local)
{
    extern int edit_entity;

    edit_entity = i;
}

#endif
