#pragma once
#include "shared/tools/vector_util.h"


class Entity;
class BaseEntity;
class MovableEntity;
class DynamicEntity;
class ModelEntity;
//class DynamicLight;
class LightEntity;
class SkeletalEntity;

// Entity arrays.
extern vector<Entity *> g_ents;
extern vector<LightEntity *> g_lightEnts;

//
// Entity core functions.
//
// Preloads the entities.
extern void preloadentities();

// Renders all the entities.
// extern void renderentities();

// Sets the spawn state on a given entity index.
extern void setspawn(int i, bool on);

// Resets all the spawns.
extern void resetspawns();

