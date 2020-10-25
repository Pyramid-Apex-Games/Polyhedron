#include "EntityFactory.h"
#include "cube.h"
#include "game.h"
#include "ents.h"
#include "Entity.h"

namespace {
	const std::string fallbackEntityType = "entity";
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(const std::monostate& value) const
{
	return "";
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(std::string const& value) const
{
	return value;
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(float value) const
{
	return std::to_string(value);
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(int value) const
{
	return std::to_string(value);
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(bool value) const
{
	return std::to_string(value);
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(vec4 value) const
{
	return std::to_string(value.x) + ", " + std::to_string(value.y) + ", " + std::to_string(value.z) + ", " + std::to_string(value.w);
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(vec value) const
{
	return std::to_string(value.x) + ", " + std::to_string(value.y) + ", " + std::to_string(value.z);
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(ivec4 value) const
{
	return std::to_string(value.x) + ", " + std::to_string(value.y) + ", " + std::to_string(value.z) + ", " + std::to_string(value.w);
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(ivec value) const
{
	return std::to_string(value.x) + ", " + std::to_string(value.y) + ", " + std::to_string(value.z);
}


template <> float AttributeVisitCoercer<float>::operator()(const std::monostate& value) const
{
	return 0.0f;
}

template <> float AttributeVisitCoercer<float>::operator()(const std::string& value) const
{
    if (value.empty())
        return 0.0f;
	return std::stof(value);
}

template <> float AttributeVisitCoercer<float>::operator()(const float value) const
{
	return value;
}

template <> float AttributeVisitCoercer<float>::operator()(const int value) const
{
	return round(value);
}

template <> float AttributeVisitCoercer<float>::operator()(const bool value) const
{
	return value ? 1.0f : 0.0f;
}

template <> float AttributeVisitCoercer<float>::operator()(const vec4 value) const
{
	return value.x;
}

template <> float AttributeVisitCoercer<float>::operator()(const vec value) const
{
	return value.x;
}

template <> float AttributeVisitCoercer<float>::operator()(const ivec4 value) const
{
	return value.x;
}

template <> float AttributeVisitCoercer<float>::operator()(const ivec value) const
{
	return value.x;
}


template <> int AttributeVisitCoercer<int>::operator()(const std::monostate& value) const
{
	return 0;
}

template <> int AttributeVisitCoercer<int>::operator()(std::string const& value) const
{
    if (value.empty())
        return 0;
	return std::stoi(value);
}

template <> int AttributeVisitCoercer<int>::operator()(float value) const
{
	return value;
}

template <> int AttributeVisitCoercer<int>::operator()(int value) const
{
	return value;
}

template <> int AttributeVisitCoercer<int>::operator()(bool value) const
{
	return value ? 1 : 0;
}

template <> int AttributeVisitCoercer<int>::operator()(vec4 value) const
{
	return value.x;
}

template <> int AttributeVisitCoercer<int>::operator()(vec value) const
{
	return value.x;
}

template <> int AttributeVisitCoercer<int>::operator()(ivec4 value) const
{
	return value.x;
}

template <> int AttributeVisitCoercer<int>::operator()(ivec value) const
{
	return value.x;
}


template <> bool AttributeVisitCoercer<bool>::operator()(const std::monostate& value) const
{
	return false;
}

template <> bool AttributeVisitCoercer<bool>::operator()(const std::string& value) const
{
    if (value.empty())
        return false;
	return value == "true" || value == "1" || value == "True" || value == "TRUE" || std::stoi(value) != 0 || std::stof(value) != 0.0f ? true : false;
}

template <> bool AttributeVisitCoercer<bool>::operator()(const float value) const
{
	return value != 0.0f ? true : false;
}

template <> bool AttributeVisitCoercer<bool>::operator()(const int value) const
{
	return value != 0 ? true : false;
}

template <> bool AttributeVisitCoercer<bool>::operator()(const bool value) const
{
	return value;
}

template <> bool AttributeVisitCoercer<bool>::operator()(const vec4 value) const
{
	return value.x != 0 ? true : false;
}

template <> bool AttributeVisitCoercer<bool>::operator()(const vec value) const
{
	return value.x != 0 ? true : false;
}

template <> bool AttributeVisitCoercer<bool>::operator()(const ivec4 value) const
{
	return value.x != 0 ? true : false;
}

template <> bool AttributeVisitCoercer<bool>::operator()(const ivec value) const
{
	return value.x != 0 ? true : false;
}


template <> vec4 AttributeVisitCoercer<vec4>::operator()(const std::monostate& value) const
{
	return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

template <> vec4 AttributeVisitCoercer<vec4>::operator()(const std::string& value) const
{
    //FIXME: implement tokenization + number extraction from string
	return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

template <> vec4 AttributeVisitCoercer<vec4>::operator()(const float value) const
{
	return vec4(value, 0.0f, 0.0f, 0.0f);
}

template <> vec4 AttributeVisitCoercer<vec4>::operator()(const int value) const
{
	return vec4(value, 0.0f, 0.0f, 0.0f);
}

template <> vec4 AttributeVisitCoercer<vec4>::operator()(const bool value) const
{
	return value ? vec4(1.0f, 1.0f, 1.0f, 1.0f) : vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

template <> vec4 AttributeVisitCoercer<vec4>::operator()(const vec4 value) const
{
	return value;
}

template <> vec4 AttributeVisitCoercer<vec4>::operator()(const vec value) const
{
	return vec4(value, 0.0f);
}

template <> vec4 AttributeVisitCoercer<vec4>::operator()(const ivec4 value) const
{
	return vec4(value.x, value.y, value.z, value.w);
}

template <> vec4 AttributeVisitCoercer<vec4>::operator()(const ivec value) const
{
	return vec4(value.x, value.y, value.z, 0.0f);
}


template <> vec AttributeVisitCoercer<vec>::operator()(const std::monostate& value) const
{
	return vec(0.0f, 0.0f, 0.0f);
}

template <> vec AttributeVisitCoercer<vec>::operator()(const std::string& value) const
{
    //FIXME: implement tokenization + number extraction from string
	return vec(0.0f, 0.0f, 0.0f);
}

template <> vec AttributeVisitCoercer<vec>::operator()(const float value) const
{
	return vec(value, 0.0f, 0.0f);
}

template <> vec AttributeVisitCoercer<vec>::operator()(const int value) const
{
	return vec(value, 0.0f, 0.0f);
}

template <> vec AttributeVisitCoercer<vec>::operator()(const bool value) const
{
	return value ? vec(1.0f, 1.0f, 1.0f) : vec(0.0f, 0.0f, 0.0f);
}

template <> vec AttributeVisitCoercer<vec>::operator()(const vec4 value) const
{
	return vec(value.x, value.y, value.z);
}

template <> vec AttributeVisitCoercer<vec>::operator()(const vec value) const
{
	return value;
}

template <> vec AttributeVisitCoercer<vec>::operator()(const ivec4 value) const
{
	return vec(value.x, value.y, value.z);
}

template <> vec AttributeVisitCoercer<vec>::operator()(const ivec value) const
{
	return vec(value.x, value.y, value.z);
}


template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const std::monostate& value) const
{
	return ivec4(0, 0, 0, 0);
}

template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const std::string& value) const
{
    //FIXME: implement tokenization + number extraction from string
	return ivec4(0, 0, 0, 0);
}

template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const float value) const
{
	return ivec4(value, 0, 0, 0);
}

template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const int value) const
{
	return ivec4(value, 0, 0, 0);
}

template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const bool value) const
{
	return value ? ivec4(1, 1, 1, 1) : ivec4(0, 0, 0, 0);
}

template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const vec4 value) const
{
	return ivec4(value.x, value.y, value.z, value.w);
}

template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const vec value) const
{
	return ivec4(value.x, value.y, value.z, 0);
}

template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const ivec4 value) const
{
	return value;
}

template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const ivec value) const
{
	return ivec4(value.x, value.y, value.z, 0.0f);
}

template <> ivec AttributeVisitCoercer<ivec>::operator()(const std::monostate& value) const
{
	return ivec(0, 0, 0);
}

template <> ivec AttributeVisitCoercer<ivec>::operator()(const std::string& value) const
{
    //FIXME: implement tokenization + number extraction from string
	return ivec(0, 0, 0);
}

template <> ivec AttributeVisitCoercer<ivec>::operator()(const float value) const
{
	return ivec(value, 0, 0);
}

template <> ivec AttributeVisitCoercer<ivec>::operator()(const int value) const
{
	return ivec(value, 0, 0);
}

template <> ivec AttributeVisitCoercer<ivec>::operator()(const bool value) const
{
	return value ? ivec(1, 1, 1) : ivec(0, 0, 0);
}

template <> ivec AttributeVisitCoercer<ivec>::operator()(const vec4 value) const
{
	return ivec(value.x, value.y, value.z);
}

template <> ivec AttributeVisitCoercer<ivec>::operator()(const vec value) const
{
	return ivec(value.x, value.y, value.z);
}

template <> ivec AttributeVisitCoercer<ivec>::operator()(const ivec4 value) const
{
	return ivec(value.x, value.y, value.z);
}

template <> ivec AttributeVisitCoercer<ivec>::operator()(const ivec value) const
{
	return value;
}



std::map<std::string, EntityFactory::EntityFactoryConstructor>& EntityFactory::getConstructors()
{
	static std::map<std::string, EntityFactoryConstructor> s_ConstructorList;
	
	return s_ConstructorList;
}


std::map<std::string, EntityFactory::EntityFactoryAttribute>& EntityFactory::getAttributors()
{
	static std::map<std::string, EntityFactoryAttribute> s_AttributorList;
	
	return s_AttributorList;
}


void EntityFactory::addEntityFactory(const std::string &classname, EntityFactory::EntityFactoryConstructor constructor, EntityFactory::EntityFactoryAttribute attributor)
{
	auto& constructorList = getConstructors();
	
	if (constructorList.find(classname) != constructorList.end())
	{
		printf("Entity Constructor Factory for class '%s' already exists!\n", classname.c_str());
		return;
	}
	
	auto& attributorList = getAttributors();
	
	if (attributorList.find(classname) != attributorList.end())
	{
		printf("Entity Attributor Factory for class '%s' already exists!\n", classname.c_str());
		return;
	}

	printf("Entity Factory for class '%s' registered\n", classname.c_str());
	constructorList[classname] = constructor;
	attributorList[classname] = attributor;
}

// Constructs the entity by using the special macro generated EntityNameConstruct function.
Entity* EntityFactory::constructEntity(const std::string &classname)
{
	auto& constructorList = getConstructors();

	if (constructorList.find(classname) == constructorList.end())
	{
		conoutf(CON_WARN, "Entity Factory for class '%s' missing!", classname.c_str());
		return constructorList.at(fallbackEntityType)();
	}
	else
	{
		conoutf("Constructed class: %s", classname.c_str());
		return constructorList.at(classname)();
	}
}


attributeList_T EntityFactory::attributes(const std::string &classname)
{
	auto& attributorList = getAttributors();

	if (attributorList.find(classname) == attributorList.end())
	{
		conoutf(CON_WARN, "Entity Factory for class '%s' missing!", classname.c_str());
		return attributorList.at(fallbackEntityType)();
	}
	else
	{
		return attributorList.at(classname)();
	}
}


SCRIPTEXPORT void loop_all_entities(ident *id, CommandTypes::Expression body)
{
    loopstart(id, stack);
    for(auto factoryPair : EntityFactory::getAttributors())
    {
		loopiter(id, stack, factoryPair.first.c_str());
		execute(body);
    }
    loopend(id, stack);
}


SCRIPTEXPORT void get_ent_attributes(char* entityname, ident *id, CommandTypes::Expression body)
{
    loopstart(id, stack);
    for(auto row : EntityFactory::attributes(entityname))
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
