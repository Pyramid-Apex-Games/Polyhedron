#pragma once
#include "shared/tools/vector_util.h"
#include <map>
#include <string>

namespace detail {
    template<typename Type, typename SignalHandler>
    struct Event {
        Event(Type type)
                : type(type) {}

        const Type type = Type::None;
        using Signal_T = SignalHandler;
        using Listener_T = typename Signal_T::Listener;
        using Listeners_T = vector<Listener_T>;

        static constexpr auto Broadcast = Signal_T::Broadcast;
        static constexpr auto SendByIndex = Signal_T::SendByIndex;
        static constexpr auto SendIf = Signal_T::SendIf;
        static constexpr auto Send = Signal_T::Send;
    };
}

//static const auto& Broadcast = Signal_T::Broadcast;
//static const auto& SendByIndex = Signal_T::SendByIndex;
//static const auto& SendIf = Signal_T::SendIf;
//static const auto& Send = Signal_T::Send;