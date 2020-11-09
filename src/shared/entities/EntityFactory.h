#pragma once
#include "shared/architecture/Attribute.h"
#include "shared/architecture/Factory.h"

typedef unsigned int uint;
struct ident;
namespace CommandTypes
{
    typedef uint* Expression;
}

class Entity;

using EntityFactory = Factory<Entity>;
using EntityFactoryAttributes = FactoryAttributes<Entity>;

#define ADD_ENTITY_TO_FACTORY(LOCALNAME, CLASSNAME) \
    FACTORY_IMPLEMENT_BASE(Entity, LOCALNAME, CLASSNAME)

#define ADD_ENTITY_TO_FACTORY_SERIALIZED(LOCALNAME, CLASSNAME, DERIVED) \
    FACTORY_IMPLEMENT(Entity, LOCALNAME, CLASSNAME, DERIVED)

#define ENTITY_FACTORY_IMPL(LOCALNAME) \
	FACTORY_DEFINE(Entity, LOCALNAME)
