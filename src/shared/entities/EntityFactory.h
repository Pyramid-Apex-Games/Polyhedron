#pragma once
#include "shared/geom/vec4.h"
#include "shared/geom/vec.h"
#include "shared/geom/ivec4.h"
#include "shared/geom/ivec.h"
#include <string>
#include <map>
#include <functional>
#include <string>
#include <variant>

typedef unsigned int uint;
struct ident;
namespace CommandTypes
{
    typedef uint* Expression;
}

using attribute_T = std::variant<std::string, float, int, bool, vec4, vec, ivec4, ivec>;
using attrubuteRow_T = std::vector<attribute_T>;
using attributeList_T = std::vector<attrubuteRow_T>;

template <typename TargetType>
struct AttributeVisitCoercer
{
    TargetType operator()(const std::string& value) const;
    TargetType operator()(float value) const;
    TargetType operator()(int value) const;
    TargetType operator()(bool value) const;
    TargetType operator()(vec4 value) const;
    TargetType operator()(vec value) const;
    TargetType operator()(ivec4 value) const;
    TargetType operator()(ivec value) const;
};

class Entity;

class EntityFactory
{
private:
public:
    typedef std::function<Entity*()> EntityFactoryConstructor;
    typedef std::function<attributeList_T()> EntityFactoryAttribute;

    static std::map<std::string, EntityFactoryConstructor>& getConstructors();
    static std::map<std::string, EntityFactoryAttribute>& getAttributors();

    template<class ET>
    static Entity* constructor()
    {
        return static_cast<Entity*>(new ET);
    }

    static void addEntityFactory(const std::string &classname, EntityFactoryConstructor constructor, EntityFactoryAttribute attributor);

    static Entity* constructEntity(const std::string &classname);
    static attributeList_T attributes(const std::string &classname);
};


#define ADD_ENTITY_TO_FACTORY(LOCALNAME, CLASSNAME) \
    const std::string LOCALNAME::classname = CLASSNAME;\
	std::string LOCALNAME::currentClassname() { return CLASSNAME; }\
	Entity *LOCALNAME::Construct()\
	{\
		return EntityFactory::constructor<LOCALNAME>(); \
	}\
	class LOCALNAME##Intializer\
	{\
	public:\
		LOCALNAME##Intializer()\
		{\
            EntityFactory::addEntityFactory(CLASSNAME, &LOCALNAME::Construct, &LOCALNAME::attributes);\
		}\
    } LOCALNAME##IntializerInstance;


#define ADD_ENTITY_TO_FACTORY_SERIALIZED(LOCALNAME, CLASSNAME, DERIVED) \
	ADD_ENTITY_TO_FACTORY(LOCALNAME, CLASSNAME) \
    extern void from_json(const nlohmann::json&, LOCALNAME&);\
    extern void to_json(nlohmann::json&, const LOCALNAME&);\
	\
	void LOCALNAME::fromJsonImpl(const nlohmann::json& document) {\
		document.at(CLASSNAME).get_to(*this);\
		DERIVED::fromJsonImpl(document);\
	}\
	void LOCALNAME::saveToJsonImpl(nlohmann::json& document) {\
		DERIVED::saveToJsonImpl(document);\
		document[CLASSNAME] = *this;\
	}\
	void LOCALNAME::setAttribute(const std::string &key, const attribute_T &value) {\
		DERIVED::setAttribute(key, value);\
		setAttributeImpl(key, value);\
	}\
	attribute_T LOCALNAME::getAttribute(const std::string &key) const {\
		attribute_T attributeValue = DERIVED::getAttribute(key);\
		if (std::holds_alternative<std::string>(attributeValue) && std::get<std::string>(attributeValue).empty()) {\
			attributeValue = getAttributeImpl(key);\
		}\
		return attributeValue;\
	}\
	void LOCALNAME::onImpl(const Event& event) {\
		on(event);\
		DERIVED::onImpl(event);\
	}\
	void LOCALNAME::renderImpl(game::RenderPass pass) {\
		DERIVED::renderImpl(pass);\
		render(pass);\
	}

#define ENTITY_FACTORY_IMPL(LOCALNAME) \
	public:\
	friend class LOCALNAME##Intializer;\
	friend void from_json(const nlohmann::json&, LOCALNAME&);\
	friend void to_json(nlohmann::json&, const LOCALNAME&);\
    static const std::string classname;\
	virtual std::string currentClassname();\
	static Entity *Construct();\
	virtual void fromJsonImpl(const nlohmann::json& document);\
    virtual void saveToJsonImpl(nlohmann::json& document);\
	virtual void setAttribute(const std::string &key, const attribute_T &value);\
	virtual attribute_T getAttribute(const std::string &key) const;\
	void setAttributeImpl(const std::string &key, const attribute_T &value);\
	attribute_T getAttributeImpl(const std::string &key) const;\
	static const attributeList_T attributes();\
	virtual void onImpl(const Event& event);\
	void on(const Event& event);\
	virtual void renderImpl(game::RenderPass pass);\
	void render(game::RenderPass pass);\
    virtual ~LOCALNAME() = default;
