#include "shared/cube.h"
#include "shared/ents.h"
#include "shared/entities/baseentity.h"
#include "shared/entities/basephysicalentity.h"
#include "shared/entities/coreentity.h"
#include "shared/entities/entityfactory.h"
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

ADD_ENTITY_TO_FACTORY(CoreEntity, "core_entity");

void entities::classes::CoreEntity::saveToJsonImpl(nlohmann::json& document)
{
	document[classname] = {};
	to_json(document[classname], *this);
}


void entities::classes::CoreEntity::saveToJson(nlohmann::json& document)
{
	document = {
		{"class", currentClassname()},
		{"et_type", et_type},
		{"ent_type", ent_type},
		{"game_type", game_type}
	};
	
	saveToJsonImpl(document);
}

void entities::classes::CoreEntity::fromJsonImpl(const nlohmann::json& document)
{
    if (document.find(classname) != document.end())
    {
	    document.at(classname).get_to(*this);
    }
}

void entities::classes::CoreEntity::loadFromJson(const nlohmann::json& document)
{
    if (document.find("et_type") != document.end())
    {
    	document.at("et_type").get_to(et_type);
    }
    if (document.find("ent_type") != document.end())
    {
        document.at("ent_type").get_to(ent_type);
    }
    if (document.find("game_type") != document.end())
    {
        document.at("game_type").get_to(game_type);
    }

	fromJsonImpl(document);
	
	on(EntityEventPrecache());
}

void entities::classes::CoreEntity::setAttribute(const std::string &key, const entities::attribute_T &value)
{
	setAttributeImpl(key, value);
}

entities::attribute_T entities::classes::CoreEntity::getAttribute(const std::string &key) const
{
	return getAttributeImpl(key);
}

bool entities::classes::CoreEntity::getBoundingBox(int entselradius, vec &minbb, vec &maxbb) const
{
	minbb = vec(o).sub(entselradius);
	maxbb = vec(o).add(entselradius+1);

	if (et_type == ET_EMPTY)
	{
		return false;
	}
	
	return true;
}

void entities::classes::CoreEntity::render(game::RenderPass pass)
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
			auto cameraEnt = dynamic_cast<CoreEntity*>(camera1);
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

void entities::classes::CoreEntity::renderForEdit()
{

}

void entities::classes::CoreEntity::renderForEditGui()
{

}

void entities::classes::CoreEntity::renderSelected(int entselradius, int entorient)
{

}


void entities::classes::CoreEntity::renderHighlight(int entselradius, int entorient, float thickness)
{

}

void entities::classes::CoreEntity::renderMoveShadow(int entselradius, int size)
{

}

void entities::classes::CoreEntity::onImpl(const Event& event)
{
	on(event);
}

void entities::classes::CoreEntity::renderImpl(game::RenderPass pass)
{
	render(pass);
}

void entities::classes::CoreEntity::on(const Event& event)
{
	if (event.type != EntityEventType::HoverStart)
	{
		conoutf("EntityEvent: %s", EntityEventTypeToStringMap.at(event.type).c_str());
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
				entities::send_entity_event(this, entities::EntityEventMoveStop());
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

void entities::send_entity_event(classes::CoreEntity* entity, const Event& event)
{
	if (entity)
	{
		entity->onImpl(event);
	}
}

void entities::send_entity_event(int entity_id, const Event& event)
{
	auto& ents = entities::getents();
	if (ents.inrange(entity_id))
	{
		send_entity_event(ents[entity_id], event);
	}
	else
	{
		conoutf("Unable send_entity_event failed: no such entity: %d", entity_id);
	}
}

void entities::classes::from_json(const nlohmann::json& document,  entities::classes::CoreEntity* entity_ptr)
{
	std::string name;
	document.get_to(name);
}

void entities::classes::to_json(nlohmann::json& document, const entities::classes::CoreEntity* entity_ptr)
{
	if (entity_ptr)
	{
		document = nlohmann::json {entity_ptr->name};
	}
	else
	{
		document = "";
	}
}



