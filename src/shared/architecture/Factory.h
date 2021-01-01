#pragma once
#include "shared/architecture/Attribute.h"
#include "engine/log.h"
#ifndef _MSC_VER
#include <experimental/type_traits>
using std::experimental::is_detected_v;
#else
//From: https://en.cppreference.com/w/cpp/experimental/is_detected
namespace detail {
template <class Default, class AlwaysVoid,
          template<class...> class Op, class... Args>
struct detector {
  using value_t = std::false_type;
  using type = Default;
};

template <class Default, template<class...> class Op, class... Args>
struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
  using value_t = std::true_type;
  using type = Op<Args...>;
};

struct nonesuch {
    ~nonesuch() = delete;
    nonesuch(nonesuch const&) = delete;
    void operator=(nonesuch const&) = delete;
};

} // namespace detail

template <template<class...> class Op, class... Args>
using is_detected = typename detail::detector<detail::nonesuch, void, Op, Args...>::value_t;

template <template<class...> class Op, class... Args>
using detected_t = typename detail::detector<detail::nonesuch, void, Op, Args...>::type;

template <class Default, template<class...> class Op, class... Args>
using detected_or = detail::detector<Default, void, Op, Args...>;

template< template<class...> class Op, class... Args >
constexpr bool is_detected_v = is_detected<Op, Args...>::value;
#endif

template <class Object_T>
class Factory
{
private:
public:
    using ObjectFactoryConstructor = std::function<Object_T*()>;

private:
    static std::map<std::string, ObjectFactoryConstructor>& GetConstructorsMut()
    {
        static std::map<std::string, ObjectFactoryConstructor> s_ConstructorList;

        return s_ConstructorList;
    }

public:
    static const std::map<std::string, ObjectFactoryConstructor>& GetConstructors()
    {
        return GetConstructorsMut();
    }

    template<class ET>
    static Object_T* Constructor()
    {
        return static_cast<Object_T*>(new ET);
    }

    static void AddObjectFactory(const std::string &classname, ObjectFactoryConstructor constructor)
    {
        auto& constructorList = GetConstructorsMut();

        if (constructorList.find(classname) != constructorList.end())
        {
            logoutf("Object Constructor Factory for class '%s' already exists!", classname.c_str());
            return;
        }

        logoutf("Object Factory for class '%s' registered", classname.c_str());
        constructorList[classname] = constructor;
    }

    static Object_T* ConstructObject(const std::string &classname)
    {
        auto& constructorList = GetConstructors();

        if (constructorList.find(classname) == constructorList.end())
        {
            logoutf(
                "Object Factory for class '%s' missing! Returning factory for %s instead.",
                classname.c_str(), constructorList.begin()->first.c_str());
            return constructorList.begin()->second();
        }
        else
        {
            logoutf("Constructed class: %s", classname.c_str());
            return constructorList.at(classname)();
        }
    }
};

template <class Object_T>
class FactoryAttributes
{
public:
    using ObjectFactoryAttribute = std::function<AttributeList_T()>;

private:
    static std::map<std::string, ObjectFactoryAttribute>& GetAttributorsMut()
    {
        static std::map<std::string, ObjectFactoryAttribute> s_AttributorList;

        return s_AttributorList;
    }

public:
    static const std::map<std::string, ObjectFactoryAttribute>& GetAttributors()
    {
        return GetAttributorsMut();
    }

    static void AddObjectFactory(const std::string &classname, ObjectFactoryAttribute attributor)
    {
        auto& attributorList = GetAttributorsMut();

        if (attributorList.find(classname) != attributorList.end())
        {
            logoutf("Object Attributor Factory for class '%s' already exists!", classname.c_str());
            return;
        }

        logoutf("Attribute Factory for class '%s' registered", classname.c_str());
        attributorList[classname] = attributor;
    }

    static AttributeList_T Attributes(const std::string &classname)
    {
        auto& attributorList = GetAttributors();

        if (attributorList.find(classname) == attributorList.end())
        {
            logoutf(
                "Object Attributor Factory for class '%s' missing! Using %s instead.",
                classname.c_str(), attributorList.begin()->first.c_str());
            return attributorList.begin()->second();
        }
        else
        {
            return attributorList.at(classname)();
        }
    }
};

template<typename T>
using AttributesMemberFunc_t = decltype( std::declval<T&>().Attributes() );

template<typename T>
constexpr bool HasAttributesMemberFunc = is_detected_v<AttributesMemberFunc_t, T>;

#define FACTORY_IMPLEMENT_BASE(BASECLASS, LOCALNAME, CLASSNAME) \
	const std::string& LOCALNAME::GetInstanceName() const { return LOCALNAME::GetClassName(); }\
	const std::string& LOCALNAME::GetClassName() { static const std::string _ = CLASSNAME; return _; }\
	BASECLASS *LOCALNAME::Construct()\
	{\
		return Factory<BASECLASS>::Constructor<LOCALNAME>(); \
	}\
	class LOCALNAME##Intializer\
	{\
	public:\
		LOCALNAME##Intializer()\
		{\
            Factory<BASECLASS>::AddObjectFactory(CLASSNAME, &LOCALNAME::Construct); \
            if constexpr (HasAttributesMemberFunc<LOCALNAME>)                                 \
                FactoryAttributes<BASECLASS>::AddObjectFactory(CLASSNAME, &LOCALNAME::Attributes);\
		}\
    } LOCALNAME##IntializerInstance;

#define FACTORY_JSON_SERIALIZABLE(LOCALNAME, CLASSNAME, DERIVED) \
    extern void from_json(const nlohmann::json&, LOCALNAME&);\
    extern void to_json(nlohmann::json&, const LOCALNAME&);\
	void LOCALNAME::FromJsonImpl(const nlohmann::json& document) {\
		document.at(CLASSNAME).get_to(*this);\
		DERIVED::FromJsonImpl(document);\
	}\
	void LOCALNAME::SaveToJsonImpl(nlohmann::json& document) {\
		DERIVED::SaveToJsonImpl(document);\
		document[CLASSNAME] = *this;\
	}

#define FACTORY_ATTRIBUTES(LOCALNAME, CLASSNAME, DERIVED) \
	void LOCALNAME::SetAttribute(const std::string &key, const Attribute_T &value) {\
		DERIVED::SetAttribute(key, value);\
		SetAttributeImpl(key, value);\
	}\
	Attribute_T LOCALNAME::GetAttribute(const std::string &key) const {\
		Attribute_T attributeValue = DERIVED::GetAttribute(key);\
		if (std::holds_alternative<std::monostate>(attributeValue)) {\
			attributeValue = GetAttributeImpl(key);\
		}\
		return attributeValue;\
	}\
	void LOCALNAME::AttributeTreeImpl(AttributeTree_T& tree) const {\
	    DERIVED::AttributeTreeImpl(tree);\
	    tree.push_back(Attributes());\
    }\
	const AttributeTree_T LOCALNAME::AttributeTree() const {\
	    AttributeTree_T tree;\
	    AttributeTreeImpl(tree);\
	    return tree;\
	}

#define FACTORY_EVENTS(LOCALNAME, CLASSNAME, DERIVED) \
	void LOCALNAME::OnImpl(const Event& event) {\
		On(event);\
		DERIVED::OnImpl(event);\
	}


#define FACTORY_DEFINE_BASE(BASECLASS, LOCALNAME) \
	public:\
	friend class LOCALNAME##Intializer;\
	static const std::string& GetClassName();\
	virtual const std::string& GetInstanceName() const;\
	static BASECLASS *Construct();

#define FACTORY_DEFINE_JSON_SERIALIZABLE(LOCALNAME) \
	public:\
	friend void from_json(const nlohmann::json&, LOCALNAME&);\
	friend void to_json(nlohmann::json&, const LOCALNAME&);  \
	virtual void FromJsonImpl(const nlohmann::json& document);\
    virtual void SaveToJsonImpl(nlohmann::json& document);

#define FACTORY_DEFINE_ATTRIBUTES(LOCALNAME) \
	virtual void SetAttribute(const std::string &key, const Attribute_T &value);\
	virtual Attribute_T GetAttribute(const std::string &key) const;\
	void SetAttributeImpl(const std::string &key, const Attribute_T &value);\
	Attribute_T GetAttributeImpl(const std::string &key) const;\
	static const AttributeList_T Attributes();\
	virtual void AttributeTreeImpl(AttributeTree_T& tree) const;\
	virtual const AttributeTree_T AttributeTree() const;

#define FACTORY_DEFINE_EVENTS(LOCALNAME) \
	virtual void OnImpl(const Event& event);\
	void On(const Event& event);

#define FACTORY_IMPLEMENT(BASECLASS, LOCALNAME, CLASSNAME, DERIVED) \
    FACTORY_IMPLEMENT_BASE(BASECLASS, LOCALNAME, CLASSNAME)         \
    FACTORY_JSON_SERIALIZABLE(LOCALNAME, CLASSNAME, DERIVED) \
    FACTORY_ATTRIBUTES(LOCALNAME, CLASSNAME, DERIVED)    \
    FACTORY_EVENTS(LOCALNAME, CLASSNAME, DERIVED)

#define FACTORY_DEFINE(BASECLASS, LOCALNAME) \
    FACTORY_DEFINE_BASE(BASECLASS, LOCALNAME)         \
    FACTORY_DEFINE_JSON_SERIALIZABLE(LOCALNAME) \
    FACTORY_DEFINE_ATTRIBUTES(LOCALNAME)    \
    FACTORY_DEFINE_EVENTS(LOCALNAME)

