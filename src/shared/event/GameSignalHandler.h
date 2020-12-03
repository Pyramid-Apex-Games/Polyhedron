#pragma once
#include "EventHandlerDetail.h"
#include "GameEventType.h"
#include "shared/entities/Entity.h"


struct GameSignalHandler :
    public GameEventHandlerBaseType
{
    void OnEvent(const GameEvent& event) const;
    void Send(const GameEvent& event, Entity& target) const;
};

