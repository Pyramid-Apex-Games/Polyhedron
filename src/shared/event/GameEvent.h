#pragma once
#include "Event.h"

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