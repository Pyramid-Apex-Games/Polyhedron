#include "game.h"
#include "engine/scriptexport.h"

#include "entities.h"
#include "entities/SkeletalEntity.h"

// This file its soul purpose is to have all CubeScript COMMAND definitions located in a single file.
//---------------------------------------------------------------------------------------------//
// VARIABLES USED BY COMMANDS.                                                                 //
//---------------------------------------------------------------------------------------------//
// COMMAND(S): ent_....
int edit_entity = -1;

namespace game {
    //---------------------------------------------------------------------------------------------//
    // ENTITY COMMANDS.                                                                            //
    //---------------------------------------------------------------------------------------------//
    // ent_set_attr - Sets the value of the designated key property.
    // args: (str)key (str)value.
    SCRIPTEXPORT void ent_set_attr(char *key, tagval *value)
    {
        auto& ents = getents();

        if (edit_entity > -1 && edit_entity < ents.length())
        {
            auto ent = ents[edit_entity];
            
            if (value->type == VAL_INT)
            {
				ent->setAttribute(key, value->getint());
			}
			else if (value->type == VAL_FLOAT)
			{
				ent->setAttribute(key, value->getfloat());
			}
			else if (value->type == VAL_STR || value->type == VAL_MACRO || value->type == VAL_CSTR)
			{
				ent->setAttribute(key, std::string(value->getstr()));
			}
			else
			{
				conoutf("Error: ent_set_attr: Unrecognized value type: %d", value->type);
			}
        }
        else
        {
			conoutf("Error: ent_set_attr: No entity selected");
        }
    }

    // ent_get_attr - Retreives the value of the designated key property.
    // args: (str)key
    SCRIPTEXPORT void ent_get_attr(char *attribute_key)
    {
        auto& ents = getents();

        if (edit_entity > -1 && edit_entity < ents.length())
        {
            if (attribute_key == nullptr || attribute_key[0] == '\0')
            {
                conoutf("missing attribute key");
                return;
            }

            auto ent = ents[edit_entity];
            auto val = ent->getAttribute(attribute_key);

			try {
				if (std::holds_alternative<std::string>(val))
				{
					result(std::get<std::string>(val).c_str());
				}
				else if (std::holds_alternative<float>(val))
				{
					floatret(std::get<float>(val));
				}
				else if (std::holds_alternative<int>(val))
				{
					intret(std::get<int>(val));
				}
				else if (std::holds_alternative<bool>(val))
				{
					intret(std::get<bool>(val) ? 1 : 0);
				}
				else
				{
					commandret->setnull();
				}
				
				return;
			}
			catch(std::bad_variant_access& e)
			{
				std::string accessError = "A_ERROR(" + std::string(e.what()) + ")";
				conoutf("Error: ent_get_attr: %s", accessError.c_str());
			}
			
        }
        else
        {
            conoutf("%s", "No valid entity selected to fetch an attribute from.");
        }
        
		commandret->setnull();
    }

    // ent_list_attr - Lists all the properties of the given entity.
    SCRIPTEXPORT void ent_list_attr(ident *id, CommandTypes::Expression body)
    {
		loopstart(id, stack);
        auto& ents = getents();

        if (edit_entity > -1 && edit_entity < ents.length())
        {
            auto ent = ents[edit_entity];

			auto attributeList = ent->attributes();
            for(auto row : attributeList)
            {
				if (row.size() > 2 && std::holds_alternative<std::string>(row[2]))
				{
					auto key = std::get<std::string>(row[2]);
					auto value = ent->getAttribute(key);
					
					try {
						if (std::holds_alternative<std::string>(value))
						{
							loopiter(id, stack, std::get<std::string>(value).c_str());
						}
						else if (std::holds_alternative<float>(value))
						{
							loopiter(id, stack, std::get<float>(value));
						}
						else if (std::holds_alternative<int>(value))
						{
							loopiter(id, stack, std::get<int>(value));
						}
						else if (std::holds_alternative<bool>(value))
						{
							loopiter(id, stack, std::get<bool>(value) ? 1 : 0);
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
        }
        else
        {
            conoutf("%s", "No valid entity selected to fetch an attribute from.");
        }
		loopend(id, stack);
    }

    SCRIPTEXPORT void gotosel()
    {
        if(player1->state!=CS_EDITING) return;
        player1->o = getselpos();
        vec dir;
        vecfromyawpitch(player1->d.x, player1->d.y, 1, 0, dir);
        player1->o.add(dir.mul(-32));
        player1->resetinterp();
    }
}
