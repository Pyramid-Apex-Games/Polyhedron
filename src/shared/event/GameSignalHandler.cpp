#include "GameSignalHandler.h"
#include "GameEvent.h"
#include "engine/main/Application.h"
#include "engine/state/State.h"
#include <type_traits>

static State* s_ActiveState = nullptr;

void GameSignalHandler::OnEvent(const GameEvent& event) const
{
    Application::Instance().GetState().OnEvent(event);
}

void GameSignalHandler::Send(const GameEvent& event, Entity& target) const
{
    target.OnImpl(event);
}

//TODO: fix the repeat, for now need it
template <>
void detail::EventHandler<
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
    >::OnEvent(const EventBase& event) const
{
    const auto& eventDerived = static_cast<const GameEvent&>(event);
    const auto& handlerDerived = static_cast<const GameSignalHandler&>(*this);
    handlerDerived.OnEvent(eventDerived);
}

template <>
void detail::EventHandler<
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
    >::Send(const EventBase& event, Entity& listener) const
{
    const auto& eventDerived = static_cast<const GameEvent&>(event);
    const auto& handlerDerived = static_cast<const GameSignalHandler&>(*this);
    handlerDerived.Send(eventDerived, listener);
}