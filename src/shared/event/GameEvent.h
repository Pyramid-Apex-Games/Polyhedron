#pragma once
#include "Event.h"
#include <map>

enum class GameEventType
{
    None,

    Start,
    Tick,
    Use,
    Shoot,
    Stop,

    Count
};

struct GameSignalHandler;
using GameEvent = detail::Event<GameEventType, GameSignalHandler>;

static const std::map<GameEventType, std::string> GameEventTypeToStringMap = {
    {GameEventType::None, "None"},
    {GameEventType::Start, "Start"},
    {GameEventType::Tick, "Tick"},
    {GameEventType::Use, "Use"},
    {GameEventType::Shoot, "Shoot"},
    {GameEventType::Stop, "Stop"},
    {GameEventType::Count, "Count"}
};

class iGame;

struct GameSignalHandler
{
    using Listener = iGame *;
    using FindListenerPredicate = std::function<bool (const Listener&)>;

    static void Broadcast(const GameEvent& event);
    static void SendByIndex(const GameEvent& event, int index) {}
    static void SendIf(const GameEvent& event, const FindListenerPredicate& target_if) {}
    static void Send(const GameEvent& event, const Listener& target) {}
};