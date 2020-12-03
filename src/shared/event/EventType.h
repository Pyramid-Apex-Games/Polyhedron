#pragma once
#include <string>
#include <map>

enum class EventType : char
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

    Load,
    Move,
    Strafe,
    Jump,
    Crouch,
    Shoot,
    Unload,

    Start,
    Stop,

    Count
};

static const std::map<EventType, std::string> EventTypeToStringMap = {
    {EventType::None, "None"},

    {EventType::Tick, "Tick"},
    {EventType::AttributeChanged, "AttributeChanged"},
    {EventType::SelectStart, "SelectStart"},
    {EventType::SelectStop, "SelectStop"},
    {EventType::MoveStart, "MoveStart"},
    {EventType::MoveStop, "MoveStop"},
    {EventType::HoverStart, "HoverStart"},
    {EventType::HoverStop, "HoverStop"},
    {EventType::TouchStart, "TouchStart"},
    {EventType::TouchStop, "TouchStop"},
    {EventType::Use, "Use"},
    {EventType::Trigger, "Trigger"},
    {EventType::Precache, "Precache"},
    {EventType::Spawn, "Spawn"},
    {EventType::ClearSpawn, "ClearSpawn"},

    {EventType::Load, "Load"},
    {EventType::Move, "Move"},
    {EventType::Strafe, "Strafe"},
    {EventType::Jump, "Jump"},
    {EventType::Crouch, "Crouch"},
    {EventType::Shoot, "Shoot"},
    {EventType::Unload, "Unload"},

    {EventType::Start, "Start"},
    {EventType::Stop, "Stop"},

    {EventType::Count, "Count"},
};