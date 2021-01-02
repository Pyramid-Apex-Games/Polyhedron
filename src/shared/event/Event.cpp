#include "Event.h"
#ifdef WIN32
detail::EventHandlerBase::EventHandlerMap_t detail::EventHandlerBase::RegisteredEventHandlers = detail::EventHandlerBase::ForEachEnumGenerator<EventType>();
#else
constinit detail::EventHandlerBase::EventHandlerMap_t detail::EventHandlerBase::RegisteredEventHandlers =
    detail::EventHandlerBase::ForEachEnumGenerator<EventType>();
#endif
void InitializeEventHandlers()
{
    static GameSignalHandler gameSignalHandler;
//    static EntitySignalHandler entitySignalHandler;
//    static StateSignalHandler stateSignalHandler;
}