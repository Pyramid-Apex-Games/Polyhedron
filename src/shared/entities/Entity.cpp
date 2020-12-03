#include "shared/cube.h"
#include "shared/ents.h"
#include "shared/entities/DynamicEntity.h"
#include "shared/entities/Entity.h"
#include "shared/entities/MovableEntity.h"
#include "shared/entities/EntityFactory.h"
#include "shared/event/Event.h"
#include "game/game.h"
#include "engine/engine.h"
#include "engine/texture.h"
#include "engine/rendergl.h"
#include "engine/world.h"
#include "engine/editor/ui.h"
#include "engine/Camera.h"
#include <vector>
#include <string>
#include <variant>

extern void boxs3D(const vec &o, vec s, int g);
extern void boxs(int orient, vec o, const vec &s);
extern void boxs(int orient, vec o, const vec &s, float size);
extern int entselradius;
extern int worldsize;

ADD_ENTITY_TO_FACTORY(Entity, "entity");

void Entity::SaveToJsonImpl(nlohmann::json& document)
{
	document[Entity::GetClassName()] = *this;
}

void Entity::SaveToJson(nlohmann::json& document)
{
	document = {
		{"class", GetInstanceName()}
	};

	SaveToJsonImpl(document);
}

void Entity::FromJsonImpl(const nlohmann::json& document)
{
    if (document.find(Entity::GetClassName()) != document.end())
    {
	    document.at(Entity::GetClassName()).get_to(*this);
    }
}

void Entity::LoadFromJson(const nlohmann::json& document)
{
	FromJsonImpl(document);
	
	On(EntityEventPrecache());
}

void Entity::SetAttribute(const std::string &key, const Attribute_T &value)
{
	SetAttributeImpl(key, value);
}

Attribute_T Entity::GetAttribute(const std::string &key) const
{
	return GetAttributeImpl(key);
}

void Entity::AttributeTreeImpl(AttributeTree_T& tree) const
{
	tree.push_back(Attributes());
}

const AttributeTree_T Entity::AttributeTree() const
{
	AttributeTree_T tree;
	AttributeTreeImpl(tree);
	return tree;
}

bool Entity::getBoundingBox(int entselradius, vec &minbb, vec &maxbb) const
{
	minbb = vec(o).sub(entselradius);
	maxbb = vec(o).add(entselradius+1);

	return true;
}

void Entity::render(RenderPass pass)
{
	if (pass == RenderPass::Edit)
	{
		ldrnotextureshader->set();
		vec bbmin;
		vec bbmax;
		getBoundingBox(entselradius, bbmin, bbmax);
		
		vec es;
		vec eo;
		eo.x = bbmin.x;
		eo.y = bbmin.y;
		eo.z = bbmin.z;
		es.x = bbmax.x - bbmin.x;
		es.y = bbmax.y - bbmin.y;
		es.z = bbmax.z - bbmin.z;
		
		if (selected)
		{
			gle::colorub(0, 40, 0);
			gle::defvertex();
			gle::begin(GL_LINES, 20*6);
			
			boxs3D(eo, es, 1);
			
			xtraverts += gle::end();
		}
		
		if (hovered)
		{
			gle::colorub(40, 40, 0);
			
			boxs3D(eo, es, 1);

			gle::colorub(200,0,0);
			const auto& cameraEnt = Camera::GetActiveCamera();
			if (cameraEnt)
			{
				float cameraRelativeTickness = clamp(0.015f*cameraEnt->o.dist(o)*tan(fovy*0.5f*RAD), 0.1f, 1.0f);
				boxs(hover_orientation, eo, es);//, cameraRelativeTickness);
			}
		}
		
		if (moving)
		{
			vec a, b;

			gle::colorub(180, 80, 80);
			(a = eo).x = eo.x - fmod(eo.x, worldsize); (b = es).x = a.x + worldsize; boxs3D(a, b, 1);
			(a = eo).y = eo.y - fmod(eo.y, worldsize); (b = es).y = a.x + worldsize; boxs3D(a, b, 1);
			(a = eo).z = eo.z - fmod(eo.z, worldsize); (b = es).z = a.x + worldsize; boxs3D(a, b, 1);
		}
	}
}

void Entity::renderForEdit()
{

}

void Entity::renderForEditGui()
{

}

void Entity::renderSelected(int entselradius, int entorient)
{

}


void Entity::renderHighlight(int entselradius, int entorient, float thickness)
{

}

void Entity::renderMoveShadow(int entselradius, int size)
{

}

void Entity::OnImpl(const Event& event)
{
	On(event);
}

void Entity::On(const Event& event)
{
	if (
        event.type != EventType::HoverStart &&
        event.type != EventType::Tick &&
        event.type != EventType::AttributeChanged
    )
	{
		conoutf(CON_DEBUG, "EntityEvent: %s", EventTypeToStringMap.at(event.type).c_str());
	}
	
	switch(event.type)
	{
		case EventType::AttributeChanged: {
		    const auto changedEvent = static_cast<const EntityEventAttributeChanged*>(&event);
		    if (changedEvent)
            {
		        if (changedEvent->payload == "o")
                {
		            if (entityId == -1)
                    {
		                int idx = 0;
		                for(idx = 0; idx < getents().size(); ++idx)
                        {
		                    if (getents()[idx] == this)
                            {
		                        break;
                            }
		                    ++idx;
                        }
		                if (idx < getents().size())
                        {
		                    entityId = idx;
                        }
                    }
		            if (entityId >= 0)
                    {
		                addentityedit(entityId);
                    }

                }
            }
		} break;
		case EventType::SelectStart:
			selected = true;
			EditorUI::StartEntityEditor(entityId);
		break;
		case EventType::SelectStop:
			selected = false;
			EditorUI::StopEntityEditor(entityId);
		break;
		case EventType::Tick:
		break;
		case EventType::Use:
		break;
		case EventType::HoverStart:{
			hovered = true;
			auto hoverEventData = static_cast<const detail::EventData<int>&>(event);
			hover_orientation = hoverEventData.payload;
			conoutf("EntityEvent for #%d: %s %d", entityId, EventTypeToStringMap.at(event.type).c_str(), hover_orientation);
		} break;
		case EventType::HoverStop:
			hovered = false;
			if (moving)
			{
				send_entity_event(this, EntityEventMoveStop());
			}
		break;
		case EventType::MoveStart:
			moving = true;
		break;
		case EventType::MoveStop:
			moving = false;
		break;
		case EventType::TouchStart:
		break;
		case EventType::TouchStop:
		break;
		case EventType::Trigger:
		break;
		case EventType::Precache:
		break;
		case EventType::Spawn:
			spawned = true;
		break;
		case EventType::ClearSpawn:
			spawned = false;
		break;

		default:
		case EventType::None:
		case EventType::Count:
		break;
	}
}

void send_entity_event(Entity* entity, const Event& event)
{
	if (entity)
	{
		entity->OnImpl(event);
	}
}

void send_entity_event(int entity_id, const Event& event)
{
	auto& ents = getents();
	if (in_range(entity_id, ents))
	{
		send_entity_event(ents[entity_id], event);
	}
	else
	{
		conoutf("Unable send_entity_event failed: no such entity: %d", entity_id);
	}
}

void from_json(const nlohmann::json& document,  Entity& entity)
{
    if (document.find("o") != document.end())
    {
    	document.at("o").get_to(entity.o);
    }
    if (document.find("d") != document.end())
    {
        document.at("d").get_to(entity.d);
    }
    if (document.find("flags") != document.end())
    {
        document.at("flags").get_to(entity.flags);
    }
    if (document.find("name") != document.end())
    {
        document.at("name").get_to(entity.name);
    }
}

void to_json(nlohmann::json& document, const Entity& entity)
{
    document = nlohmann::json {
        {"name", entity.name},
        {"o", entity.o},
        {"d", entity.d},
        {"flags", entity.flags},
    };
}



