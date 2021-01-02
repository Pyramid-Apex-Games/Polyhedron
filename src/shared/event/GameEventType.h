#pragma once
#include <map>
#include <string>

class State;
struct GameSignalHandler;
struct GameEvent;
struct GameSignalHandler;
class Entity;

using GameEventHandlerBaseType =
    detail::EventHandler<
        GameSignalHandler,
        GameEvent,
        Entity,
        EventType::Load,
        EventType::Tick,
        EventType::Move,
        EventType::Strafe,
        EventType::Jump,
        EventType::Crouch,
        EventType::Use,
        EventType::Shoot,
        EventType::Unload
    >;