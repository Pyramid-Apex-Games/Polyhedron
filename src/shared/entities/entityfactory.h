#pragma once
#include <string>
#include <map>
#include <functional>

namespace entities {

	namespace classes
	{
		class CoreEntity;
	}

    //
    // EntityFactory class used to manage entities. Entities are linked to it by the macro:
    // Example: ADD_ENTITY_TO_FACTORY(BaseDynamicEntity, "base_dynamic_entity");
    //
    // Always place this at the bottom of your entityname.cpp file and it'll automatically
    // register itself as a possible entity to spawn in your maps.
    //
    class EntityFactory
    {
    private:
        // List of our factory functions.
        //static std::map<std::string, std::function<entities::classes::CoreEntity*()> > factoryList;
        typedef std::function<entities::classes::CoreEntity*()> EntityFactoryConstructor;
        
        // Factory functions storage
        static std::map<std::string, EntityFactoryConstructor>& getFactories();

    public:        
        template<class ET>
        static entities::classes::CoreEntity* constructor()
        {
			return static_cast<entities::classes::CoreEntity*>(new ET);
		}

        static void addEntityFactory(const std::string &classname, EntityFactoryConstructor constructor);
        static entities::classes::CoreEntity* constructEntity(const std::string &classname);
    };
}


//
// Use ADD_ENTITY_TO_FACTORY at the bottom of your entityname.cpp file to hook it up to a
// string name. For example: ADD_ENTITY_TO_FACTORY(GarageDoor, "door_garage");
//
// LOCALNAME: The class name in the C++ code.
// CLASSNAME: The string class name used by map editing.
//
// From here on it'll be spawnable by the NewEntity function.
//

// Removed the const from the classname string so we can actually set its value (Otherwise it'd always be core_entity).
#define ADD_ENTITY_TO_FACTORY(LOCALNAME, CLASSNAME) \
    const std::string entities::classes::LOCALNAME::classname = CLASSNAME;\
	std::string entities::classes::LOCALNAME::currentClassname() { return CLASSNAME; }\
	entities::classes::CoreEntity *entities::classes::LOCALNAME::Construct()\
	{\
		return entities::EntityFactory::constructor<entities::classes::LOCALNAME>(); \
	}\
	class LOCALNAME##Intializer\
	{\
	public:\
		LOCALNAME##Intializer()\
		{\
            entities::EntityFactory::addEntityFactory(CLASSNAME, &entities::classes::LOCALNAME::Construct);\
		}\
    } LOCALNAME##IntializerInstance;

#define ADD_ENTITY_TO_FACTORY_SERIALIZED(LOCALNAME, CLASSNAME, DERIVED) \
	ADD_ENTITY_TO_FACTORY(LOCALNAME, CLASSNAME) \
	namespace entities { namespace classes {\
		extern void from_json(const nlohmann::json&, LOCALNAME&);\
		extern void to_json(nlohmann::json&, const LOCALNAME&);\
	} }\
	void entities::classes::LOCALNAME::fromJsonImpl(const nlohmann::json& document) {\
		DERIVED::fromJsonImpl(document);\
		document.at(CLASSNAME).get_to(*this);\
	}\
	void entities::classes::LOCALNAME::saveToJsonImpl(nlohmann::json& document) {\
		DERIVED::saveToJsonImpl(document);\
		document[CLASSNAME] = nlohmann::json{*this};\
	}

//
// Prepares a class capable of being spawned by the entity factory by adding the
// required members and functions. NOTE: Always ensure you add this at the top
// of your class, per example:
//
// class BoringTree : public ModelEntity {
// ENTITY_FACTORY_IMPL(ModelEntity);
//
#define ENTITY_FACTORY_IMPL(LOCALNAME) \
	public:\
	friend class LOCALNAME##Intializer;\
	friend void from_json(const nlohmann::json&, LOCALNAME&);\
	friend void to_json(nlohmann::json&, const LOCALNAME&);\
    static const std::string classname;\
	virtual std::string currentClassname();\
	static CoreEntity *Construct();\
	virtual void fromJsonImpl(const nlohmann::json& document);\
    virtual void saveToJsonImpl(nlohmann::json& document); \
    virtual ~LOCALNAME() = default;
