#pragma once
#include "Event.h"
//#include "shared/entities/Entity.h"
#include "shared/geom/vec.h"
#include <functional>

class Entity;

enum class EntityEventType
{
    None,

    Tick,
    AttributeChanged,
    SelectStart,
    SelectStop,
    MoveStart,
    MoveStop,
    HoverStart,
    HoverStop,
    TouchStart,
    TouchStop,
    Use,
    Trigger,
    Precache,
    Spawn,
    ClearSpawn,

    Count
};

struct EntitySignalHandler;
using Event = detail::Event<EntityEventType, EntitySignalHandler>;

static const std::map<EntityEventType, std::string> EntityEventTypeToStringMap = {
    {EntityEventType::None, "None"},
    {EntityEventType::Tick, "Tick"},
    {EntityEventType::AttributeChanged, "AttributeChanged"},
    {EntityEventType::SelectStart, "SelectStart"},
    {EntityEventType::SelectStop, "SelectStop"},
    {EntityEventType::MoveStart, "MoveStart"},
    {EntityEventType::MoveStop, "MoveStop"},
    {EntityEventType::HoverStart, "HoverStart"},
    {EntityEventType::HoverStop, "HoverStop"},
    {EntityEventType::TouchStart, "TouchStart"},
    {EntityEventType::TouchStop, "TouchStop"},
    {EntityEventType::Use, "Use"},
    {EntityEventType::Trigger, "Trigger"},
    {EntityEventType::Precache, "Precache"},
    {EntityEventType::Spawn, "Spawn"},
    {EntityEventType::ClearSpawn, "ClearSpawn"},
    {EntityEventType::Count, "Count"}
};

struct EntitySignalHandler
{
    using Listener = Entity *;
    using FindListenerPredicate = std::function<bool (const Listener&)>;

    static void Broadcast(const Event& event);
    static void SendByIndex(const Event& event, int index);
    static void SendIf(const Event& event, const FindListenerPredicate& target_if);
    static void Send(const Event& event, const Listener& target);
};

template <EntityEventType E>
struct EntityEvent
    : public Event
{
    EntityEvent()
        : Event(E)
    {}
};

template <EntityEventType E, typename T>
struct EntityEventData
    : public EntityEvent<E>
{
    EntityEventData(const T& payload)
        : EntityEvent<E>()
        , payload(payload)
    {}

    const T payload;
};

struct EntityEventAttributeChanged
    : public EntityEventData<EntityEventType::AttributeChanged, std::string>
{
    EntityEventAttributeChanged(const std::string& key)
        : EntityEventData<EntityEventType::AttributeChanged, std::string>(key)
    {}
};

struct EntityEventSelectStart
    : public EntityEvent<EntityEventType::SelectStart>
{
};

struct EntityEventSelectStop
    : public EntityEvent<EntityEventType::SelectStop>
{
};

struct EntityEventMoveStart
    : public EntityEvent<EntityEventType::MoveStart>
{
};

struct EntityEventMoveStop
    : public EntityEvent<EntityEventType::MoveStop>
{
};

struct EntityEventTouchStart
    : public EntityEventData<EntityEventType::TouchStart, vec>
{
    EntityEventTouchStart(const vec& val)
    : EntityEventData<EntityEventType::TouchStart, vec>(val)
    {}
};

struct EntityEventTouchStop
    : public EntityEvent<EntityEventType::TouchStop>
{
};

struct EntityEventHoverStart
    : public EntityEventData<EntityEventType::HoverStart, int>
{
    EntityEventHoverStart(int orient)
        : EntityEventData<EntityEventType::HoverStart, int>(orient)
    {}
};

struct EntityEventHoverStop
    : public EntityEvent<EntityEventType::HoverStop>
{
};

struct EntityEventPrecache
    : public EntityEvent<EntityEventType::Precache>
{
};

struct EntityEventSpawn
    : public EntityEvent<EntityEventType::Spawn>
{
};

struct EntityEventClearSpawn
    : public EntityEvent<EntityEventType::ClearSpawn>
{
};

struct EntityEventTick
    : public EntityEvent<EntityEventType::Tick>
{
};