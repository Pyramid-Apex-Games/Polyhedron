#pragma once
#include "Event.h"
#include "shared/geom/vec.h"
#include <functional>

struct EntityEvent
    : public Event
{
    EntityEvent(EventType E)
        : Event(E)
    {}
};

template <typename T>
struct EntityEventData
    : public EntityEvent
{
    EntityEventData(EventType E, const T& payload)
        : EntityEvent(E)
        , payload(payload)
    {}

    const T payload;
};

struct EntityEventAttributeChanged
    : public EntityEventData<std::string>
{
    EntityEventAttributeChanged(const std::string& key)
        : EntityEventData<std::string>(EventType::AttributeChanged, key)
    {}
};

struct EntityEventSelectStart
    : public EntityEvent
{
    EntityEventSelectStart() : EntityEvent(EventType::SelectStart) {}
};

struct EntityEventSelectStop
    : public EntityEvent
{
    EntityEventSelectStop() : EntityEvent(EventType::SelectStop) {}
};

struct EntityEventMoveStart
    : public EntityEvent
{
    EntityEventMoveStart() : EntityEvent(EventType::MoveStart) {}
};

struct EntityEventMoveStop
    : public EntityEvent
{
    EntityEventMoveStop() : EntityEvent(EventType::MoveStop) {}
};

struct EntityEventTouchStart
    : public EntityEventData<vec>
{
    EntityEventTouchStart(const vec& val)
    : EntityEventData(EventType::TouchStart, val)
    {}
};

struct EntityEventTouchStop
    : public EntityEvent
{
    EntityEventTouchStop() : EntityEvent(EventType::TouchStop) {}
};

struct EntityEventHoverStart
    : public EntityEventData<int>
{
    EntityEventHoverStart(int orient)
        : EntityEventData(EventType::HoverStart, orient)
    {}
};

struct EntityEventHoverStop
    : public EntityEvent
{
    EntityEventHoverStop() : EntityEvent(EventType::HoverStop) {}
};

struct EntityEventPrecache
    : public EntityEvent
{
    EntityEventPrecache() : EntityEvent(EventType::Precache) {}
};

struct EntityEventSpawn
    : public EntityEvent
{
    EntityEventSpawn() : EntityEvent(EventType::Spawn) {}
};

struct EntityEventClearSpawn
    : public EntityEvent
{
    EntityEventClearSpawn() : EntityEvent(EventType::ClearSpawn) {}
};

struct EntityEventTick
    : public EntityEvent
{
    EntityEventTick() : EntityEvent(EventType::Tick) {}
};