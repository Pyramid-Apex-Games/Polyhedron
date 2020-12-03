#pragma once
#include "shared/tools/vector_util.h"
#include "EventType.h"
#include <variant>
#include <cassert>
#include <experimental/coroutine>
#include <cx_map.h>
#include <cx_vector.h>
#if 0
//What I want
//From anywhere in the project being able to define a EventHandler which
//  * Binds a group of EventType
//  * Binds a listener type
//  * Forwards calls to the EventHandler
//      * Optionally with a (list of) listener type(s)
//  * Automatically cast the base Event to the base Event type of the Handler
//  * During compile-time
//  * An EventType can have multiple handlers

//Example API
    struct MyPlayerEventHandler : public EventHandler<
        PlayerEvent,                                            //Lowest base Event for *this* handler
        Entity,                                                 //listener type (the type which receives the events)
        EventType::Tick, EventType::Shoot, EventType::Move>     //Bound to these events
    {
        void Broadcast(const PlayerEvent& event);               //Send to all handlers
        void Send(const PlayerEvent& event, Entity& listener);  //Send to single handler
    };

//Example usage
    //Define and event
    struct PlayerTickEvent : public EventData<EventType::Tick, float>
    {
        PlayerTickEvent(float deltaTime)
            : EventData<EventType::Tick, float>(deltaTime)
        {}
    };

    //Define another event, without a payload
    struct PlayerShootEvent : public Event<EventType::Tick>
    {
    };

    //Trigger the events
    void Update(float deltaTime)
    {
        Event::Broadcast(PlayerTickEvent(deltaTime));
        //This calls MyPlayerEventHandler::Broadcast

        if (OnMouseButtonLeftDown)
        {
            Event::Send(PlayerShootEvent(), PlayerInstance);
            //This calls MyPlayerEventHandler::Send

            /*alternative for events without a payload
            Event::Send(EventType::Shoot, PlayerInstance);
             */
        }
    }
#endif

namespace detail {
    struct EventBase {
        constexpr EventBase(const EventType& t) : type(t) {}
        const EventType type;
    };


    struct EventHandlerBase;
    struct EventHandlerBase
    {
        struct EventTypeCmp
        {
            using is_transparent = void;
            constexpr bool operator()(const EventType l, const EventType r) const
            {
                return (int)l == (int)r;
            }
        };

        static constexpr auto EventTypeElementCount = static_cast<typename std::underlying_type<EventType>::type>(EventType::Count);
        using EventHandlerMap_t = cx::map<
            EventType,
            cx::vector<EventHandlerBase*, 4>,
            (size_t)EventTypeElementCount,
            EventTypeCmp
        >;
        template <class Enum, int Start = 0>
        static constexpr EventHandlerMap_t ForEachEnumGenerator() noexcept
        {
            EventHandlerMap_t map;
            for (int i = Start; i < EventTypeElementCount; ++i)
            {
                map[static_cast<Enum>(i++)] = EventHandlerMap_t::mapped_type {};
            }
            return map;
        }

        static constinit EventHandlerMap_t RegisteredEventHandlers;

        virtual void OnEvent(const EventBase& event) const = 0;

        static constexpr void Broadcast(const EventBase& event)
        {
            for (const auto& handler : RegisteredEventHandlers.at(event.type))
            {
                handler->OnEvent(event);
            }
        }

//        template <class Listener>
//        static constexpr void Send(const EventBase& event, const Listener& listener)
//        {
//            for (const auto& handler : RegisteredEventHandlers.at(event.type))
//            {
//                handler->Broadcast(event);
//            }
//        }
    };

    struct Event : public EventBase {
        constexpr Event(EventType E) : EventBase(E) {}
        virtual ~Event(){};

        static constexpr void Broadcast(const EventBase& event)
        {
            EventHandlerBase::Broadcast(event);
        }

//        template <class T>
//        static auto Send(const Event& event, const T& listener)
//        {
//            EventHandler::Send(std::forward<const EventBase>(event), std::forward<T>(listener));
//        }

    };

    template <typename T>
    struct EventData
        : public Event
    {
        EventData(EventType E, const T& payload)
            : Event(E)
            , payload(payload)
        {}

        const T payload;
    };


}
