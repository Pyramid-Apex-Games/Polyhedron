#pragma once
#include "EventDetail.h"
#include "Event.h"
#include <map>
#include <functional>

//using GameEvent = detail::Event<std::pair<GameEventType, GameSignalHandler> >;

struct GameEvent
    : public Event
{
    GameEvent(EventType E)
        : Event(E)
    {}
};

template <typename T>
struct GameEventData
    : public detail::EventData<T>
{
    GameEventData(EventType E, const T& payload)
        : detail::EventData<T>(E, payload)
    {}
};

struct GameEventLoad
    : public GameEvent
{
    GameEventLoad() : GameEvent(EventType::Load) {}
};

struct GameEventTick
    : public GameEvent
{
    GameEventTick(EventType E) : GameEvent(E) {}
};

struct GameEventMove
: public GameEventData<int>
{
    GameEventMove(int axis) : GameEventData(EventType::Move, axis)
    {}
};

struct GameEventStrafe
: public GameEventData<int>
{
    GameEventStrafe(int axis) : GameEventData(EventType::Strafe, axis)
    {}
};

struct GameEventJump
: public GameEventData<bool>
{
    GameEventJump(bool pressed) : GameEventData(EventType::Jump, pressed)
    {}
};

struct GameEventCrouch
: public GameEventData<bool>
{
    GameEventCrouch(bool pressed) : GameEventData(EventType::Crouch, pressed)
    {}
};


struct GameEventShoot
    : public GameEvent
{
    GameEventShoot() : GameEvent(EventType::Shoot) {}
};

struct GameEventUnload
    : public GameEvent
{
    GameEventUnload() : GameEvent(EventType::Unload) {}
};
