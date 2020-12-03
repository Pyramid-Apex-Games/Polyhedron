#pragma once
#include <functional>

class Entity;
//
//struct EntitySignalHandler
//{
//    using Listener = Entity *;
//    using FindListenerPredicate = std::function<bool (const Listener&)>;
//
//    static void Broadcast(const detail::EventBase& event);
//    static void SendByIndex(const detail::EventBase& event, int index);
//    static void SendIf(const detail::EventBase& event, const FindListenerPredicate& target_if);
//    static void Send(const detail::EventBase& event, const Listener& target);
//};