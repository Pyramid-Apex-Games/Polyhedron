#include "EntityEvent.h"
#include "engine/console.h"
#include "game/entities.h"
#include "shared/entities/Entity.h"

extern vector<Entity *> &getents();

/* void EntitySignalHandler::Broadcast(const detail::EventBase& event)
{
    for (const auto& ent : getents())
    {
        Send(event, ent);
    }
}

void EntitySignalHandler::SendByIndex(const detail::EventBase& event, int index)
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

void EntitySignalHandler::SendIf(const detail::EventBase& event, const FindListenerPredicate& target_if)
{
    for (const auto& ent : getents())
    {
        if (target_if(ent))
        {
            Send(event, ent);
        }
    }
}

void EntitySignalHandler::Send(const detail::EventBase& event, Entity* const & target)
{
    if (target)
	{
		//target->OnImpl(event);
	}
}
*/
//bool operator==(const detail::EventBase::event_type_t ve, const EntityEventType e)
//{
//    if (!std::holds_alternative<EntityEventType>(ve))
//        return false;
//
//    return std::get<EntityEventType>(ve) == e;
//}