#include "GameEvent.h"
#include "game/game.h"

static iGame* s_ActiveGame = nullptr;

void GameSignalHandler::Broadcast(const GameEvent& event)
{
    if (s_ActiveGame)
    {
        s_ActiveGame->OnEvent(event);
    }
}
