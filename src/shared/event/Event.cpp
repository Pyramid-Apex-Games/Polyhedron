#include "Event.h"

//constexpr detail::EventHandlerBase::EventHandlerMap_t detail::EventHandlerBase::RegisteredEventHandlers = detail::EventHandlerBase::ForEachEnumGenerator<EventType>();
constinit detail::EventHandlerBase::EventHandlerMap_t detail::EventHandlerBase::RegisteredEventHandlers =
    detail::EventHandlerBase::ForEachEnumGenerator<EventType>();
void InitializeEventHandlers()
{
    static GameSignalHandler gameSignalHandler;
//    static EntitySignalHandler entitySignalHandler;
//    static StateSignalHandler stateSignalHandler;
}