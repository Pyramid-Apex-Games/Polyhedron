#ifndef ENTS_H
#define ENTS_H

// This file defines static map entities ("entity") and includes the basic entities:
// (dynamic entities (players/monsters, "dynent"), and static(No AI, or Input) entities(barrels, etc, "physent")
// the gamecode extends from the BaseEntity class type (Which extends from the Entity type.)
// NOTE: dynent and physent are old Tesseract names for what now are, BaseDynamicEntity and BasePhysicalEntity

// ET_*: The internal engine entity type.
// (ent->et_type = ..)

// Include our base entity types.
//#include "entities/coreentity.h"
//#include "entities/baseentity.h"
//#include "entities/basephysicalentity.h"    // Include the physical entity, for objects such as crates etc.
//#include "entities/basedynamicentity.h"     // Include the dynamical entity, for objects that can move as if it were players.

enum {
    ET_EMPTY=0,         // Empty entities. Not in use. (No functionality, usually removed, or yet to be replaced.)
    ET_LIGHT,           // Light entities, can be dynamic lights(flickering, moving, on/off)
    ET_MAPMODEL,        // MapModel entities, can be benches, boxes, doors, anything alike.
    ET_PLAYERSTART,     // Playerstart entities, where players can spawn.
    ET_ENVMAP,          // Environment Map entity.
    ET_PARTICLES,       // Particle Effect entities.
    ET_SOUND,           // Sound Effect entities.
    ET_SPOTLIGHT,       // Spotlight entity, has to be attached to an ET_LIGHT entity.
    ET_DECAL,           // Decal entities, speak for itself.s
    ET_GAMESPECIFIC     // An entity that is specific to the game its own entity classes.
};

#define MAXENTS 10000


// Client/Server entity states. (ent->state, ent->editstate).
enum { CS_ALIVE = 0, CS_DEAD, CS_SPAWNING, CS_LAGGED, CS_EDITING, CS_SPECTATOR };

// Physics entity states. (ent->physstate)
enum { PHYS_FLOAT = 0, PHYS_FALL, PHYS_SLIDE, PHYS_SLOPE, PHYS_FLOOR, PHYS_STEP_UP, PHYS_STEP_DOWN, PHYS_BOUNCE };

// Game entity type, is it a player, AI, Inanimate, camera, or bounces.. (ent->ent_type)
enum { ENT_PLAYER = 0, ENT_AI, ENT_INANIMATE, ENT_CAMERA, ENT_BOUNCE };

// Physics collision types. (ent->collidetype)
enum { COLLIDE_NONE = 0, COLLIDE_ELLIPSE, COLLIDE_OBB, COLLIDE_TRI };

enum                              // static entity types
{
    NOTUSED = ET_EMPTY,           // entity slot not in use in map
    LIGHT = ET_LIGHT,             // lightsource, attr1 = radius, attr2 = intensity
    MAPMODEL = ET_MAPMODEL,       // modelfilename = attr1 index, attr2 = yaw, attr3 = pitch, attr4 = roll, attr5 = scale
    PLAYERSTART = ET_PLAYERSTART, // attr1 = angle, attr2 = team
    ENVMAP = ET_ENVMAP,           // attr1 = radius
    PARTICLES = ET_PARTICLES,
    MAPSOUND = ET_SOUND,
    SPOTLIGHT = ET_SPOTLIGHT,
    DECAL = ET_DECAL,

	// SchizoMania entity types. (game_type values, all should be >= ET_GAMEENTITY
	// Anyhting >= GAMEENTITY usese classname is...
	GAMEENTITY = ET_GAMESPECIFIC, // classname = game entity class type, attributes list is what it is, and can be accessed in any derived BaseEntity class.
	PLAYER,
	NPC_BASIC,
	AI_BASIC,

	// Objects.
	BUTTON,
	MAXENTTYPES,

	//new entities
	DOOR,

    I_FIRST = 0,
    I_LAST = -1
};

// hardcoded sounds, defined in sounds.cfg
enum
{
    S_JUMP = 0, S_LAND,
    S_SPLASHIN, S_SPLASHOUT, S_BURN,
    S_ITEMSPAWN, S_TELEPORT, S_JUMPPAD,
    S_MELEE, S_PULSE1, S_PULSE2, S_PULSEEXPLODE, S_RAIL1, S_RAIL2,
    S_WEAPLOAD, S_NOAMMO, S_HIT,
    S_PAIN1, S_PAIN2, S_DIE1, S_DIE2,

    S_FLAGPICKUP,
    S_FLAGDROP,
    S_FLAGRETURN,
    S_FLAGSCORE,
    S_FLAGRESET,
    S_FLAGFAIL
};


// Crouche Time, and Crouch Height. TODO: Place in Physics or player settings?
#define CROUCHTIME 200
#define CROUCHHEIGHT 0.75f

enum EntityFlags : int {
    EF_NOFLAG     = 0,
    EF_NOVIS      = 1<<0,
    EF_NOSHADOW   = 1<<1,
    EF_NOCOLLIDE  = 1<<2,
    EF_ANIM       = 1<<3,
    EF_SHADOWMESH = 1<<4,
    EF_OCTA       = 1<<5,
    EF_RENDER     = 1<<6,
    EF_SOUND      = 1<<7,
    EF_SPAWNED    = 1<<8
};


#endif // ENTS_H
