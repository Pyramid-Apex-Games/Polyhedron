#pragma once
#include "EventDetail.h"

namespace detail
{

    template <
        class Derived,
        class LocalBaseEvent_t,
        class Listener_t,
        EventType... RegisteredEvent_ts
    >
//        requires Derived::Broadcast(const LocalBaseEvent_t&);
    struct EventHandler : public EventHandlerBase
    {
        constexpr EventHandler()
        {
            for(auto& evt : {RegisteredEvent_ts...})
            {
                auto& keyVal = EventHandlerBase::RegisteredEventHandlers[evt];

                keyVal.push_back(this);
            }
        }

        void OnEvent(const EventBase& event) const override;
        void Send(const EventBase& event, Listener_t& listener) const;
    };
}

//template <
//    class Derived,
//    class LocalBaseEvent_t,
//    class Listener_t,
//    EventType... RegisteredEvent_ts
//>
//constexpr void detail::EventHandler::OnEvent(const EventBase& event) const
//{
//    const auto eventLocal = static_cast<const LocalBaseEvent_t&>(event);
//    const auto derivedThis = static_cast<const Derived*>(this);
//    derivedThis->OnEvent(eventLocal);
//}
//
//constexpr void detail::EventHandler::Send(const EventBase& event, const Listener_t& listener) const
//{
//    const auto eventLocal = static_cast<const LocalBaseEvent_t&>(event);
//    const auto derivedThis = static_cast<const Derived*>(this);
//    derivedThis->Send(eventLocal, listener);
//}