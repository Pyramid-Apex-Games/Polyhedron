#include "EntityEvent.h"
#include "engine/console.h"
#include "game/entities.h"
#include "shared/entities/Entity.h"

extern vector<Entity *> &getents();

void EntitySignalHandler::Broadcast(const Event& event)
{
    for (const auto& ent : getents())
    {
        Send(event, ent);
    }
}

void EntitySignalHandler::SendByIndex(const Event& event, int index)
{
    const auto& ents = getents();
    if (in_range(index, ents))
    {
        Send(event, ents[index]);
    }
    else
    {
        conoutf("EntitySignalHandler::SendByIndex: no such index: %d", index);
    }
}

void EntitySignalHandler::SendIf(const Event& event, const FindListenerPredicate& target_if)
{
    for (const auto& ent : getents())
    {
        if (target_if(ent))
        {
            Send(event, ent);
        }
    }
}

void EntitySignalHandler::Send(const Event& event, Entity* const & target)
{
    if (target)
	{
		target->OnImpl(event);
	}
}