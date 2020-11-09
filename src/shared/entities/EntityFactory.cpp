#include "EntityFactory.h"
#include "cube.h"
#include "game.h"
#include "ents.h"
#include "Entity.h"

namespace {
	const std::string fallbackEntityType = "entity";
}

SCRIPTEXPORT void loop_all_entities(ident *id, CommandTypes::Expression body)
{
    loopstart(id, stack);
    for(auto factoryPair : EntityFactoryAttributes::GetAttributors())
    {
		loopiter(id, stack, factoryPair.first.c_str());
		execute(body);
    }
    loopend(id, stack);
}

SCRIPTEXPORT void get_ent_attributes(char* entityname, ident *id, CommandTypes::Expression body)
{
    loopstart(id, stack);
    for(auto row : EntityFactoryAttributes::Attributes(entityname))
    {
		for(auto col : row)
		{
			try {
				if (std::holds_alternative<std::string>(col))
				{
					loopiter(id, stack, std::get<std::string>(col).c_str());
				}
				else if (std::holds_alternative<float>(col))
				{
					loopiter(id, stack, std::get<float>(col));
				}
				else if (std::holds_alternative<int>(col))
				{
					loopiter(id, stack, std::get<int>(col));
				}
				else if (std::holds_alternative<bool>(col))
				{
					loopiter(id, stack, std::get<bool>(col) ? 1 : 0);
				}
				else
				{
					loopiter(id, stack, nullptr);
				}
			}
			catch(std::bad_variant_access& e)
			{
				std::string accessError = "A_ERROR(" + std::string(e.what()) + ")";
				loopiter(id, stack, accessError.c_str());
			}
			execute(body);
		}
    }
    loopend(id, stack);
}
