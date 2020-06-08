#include "shared/cube.h"
#include "shared/ents.h"
#include "shared/entities/DynamicEntity.h"
#include "shared/entities/Entity.h"
#include "shared/entities/MovableEntity.h"
#include "shared/entities/EntityFactory.h"
#include "game/game.h"
#include "engine/engine.h"
#include "engine/texture.h"
#include "engine/rendergl.h"
#include "engine/nui/nui.h"
#include <vector>
#include <string>
#include <variant>

extern void boxs3D(const vec &o, vec s, int g);
extern void boxs(int orient, vec o, const vec &s);
extern void boxs(int orient, vec o, const vec &s, float size);
extern int entselradius;
extern int worldsize;

ADD_ENTITY_TO_FACTORY(Entity, "entity");

void Entity::saveToJsonImpl(nlohmann::json& document)
{
	document[classname] = {};
	to_json(document[classname], *this);
}


void Entity::saveToJson(nlohmann::json& document)
{
	document = {
		{"class", currentClassname()}
	};

	saveToJsonImpl(document);
}

void Entity::fromJsonImpl(const nlohmann::json& document)
{
    if (document.find(classname) != document.end())
    {
	    document.at(classname).get_to(*this);
    }
}

void Entity::loadFromJson(const nlohmann::json& document)
{
	fromJsonImpl(document);
	
	on(EntityEventPrecache());
}

void Entity::setAttribute(const std::string &key, const attribute_T &value)
{
	setAttributeImpl(key, value);
}

attribute_T Entity::getAttribute(const std::string &key) const
{
	return getAttributeImpl(key);
}

const void Entity::attributeTreeImpl(attributeTree_T& tree)
{
	tree.push_back(attributes());
}

const attributeTree_T Entity::attributeTree()
{
	attributeTree_T tree;
	attributeTreeImpl(tree);
	return tree;
}

bool Entity::getBoundingBox(int entselradius, vec &minbb, vec &maxbb) const
{
	minbb = vec(o).sub(entselradius);
	maxbb = vec(o).add(entselradius+1);

	return true;
}

void Entity::render(game::RenderPass pass)
{
	if (pass == game::RenderPass::Edit)
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
			auto cameraEnt = dynamic_cast<Entity*>(camera1);
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

void Entity::onImpl(const Event& event)
{
	on(event);
}

void Entity::renderImpl(game::RenderPass pass)
{
	render(pass);
}

void Entity::on(const Event& event)
{
	if (
        event.type != EntityEventType::HoverStart &&
        event.type != EntityEventType::Tick
    )
	{
		conoutf(CON_DEBUG, "EntityEvent: %s", EntityEventTypeToStringMap.at(event.type).c_str());
	}
	
	switch(event.type)
	{
		case EntityEventType::AttributeChanged:
		break;
		case EntityEventType::SelectStart:
			selected = true;
			engine::nui::StartEntityEditor(this);
		break;
		case EntityEventType::SelectStop:
			selected = false;
			engine::nui::StopEntityEditor(this);
		break;
		case EntityEventType::Tick:
		break;
		case EntityEventType::Use:
		break;
		case EntityEventType::HoverStart:{
			hovered = true;
			auto hoverEventData = static_cast<const EntityEventData<EntityEventType::HoverStart, int>&>(event);
			hover_orientation = hoverEventData.payload;
			conoutf("EntityEvent: %s %d", EntityEventTypeToStringMap.at(event.type).c_str(), hover_orientation);
		} break;
		case EntityEventType::HoverStop:
			hovered = false;
			if (moving)
			{
				send_entity_event(this, EntityEventMoveStop());
			}
		break;
		case EntityEventType::MoveStart:
			moving = true;
		break;
		case EntityEventType::MoveStop:
			moving = false;
		break;
		case EntityEventType::TouchStart:
		break;
		case EntityEventType::TouchStop:
		break;
		case EntityEventType::Trigger:
		break;
		case EntityEventType::Precache:
		break;
		case EntityEventType::Spawn:
			spawned = true;
		break;
		case EntityEventType::ClearSpawn:
			spawned = false;
		break;

		default:
		case EntityEventType::None:
		case EntityEventType::Count:
		break;
	}
}

void send_entity_event(Entity* entity, const Event& event)
{
	if (entity)
	{
		entity->onImpl(event);
	}
}

void send_entity_event(int entity_id, const Event& event)
{
	auto& ents = getents();
	if (ents.inrange(entity_id))
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



