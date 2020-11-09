#include "Factory.h"


//
//
//void Factory::addEntityFactory(const std::string &classname, Factory::EntityFactoryConstructor constructor, Factory::EntityFactoryAttribute attributor)
//{
//	auto& constructorList = getConstructors();
//
//	if (constructorList.find(classname) != constructorList.end())
//	{
//		printf("Entity Constructor Factory for class '%s' already exists!\n", classname.c_str());
//		return;
//	}
//
//	auto& attributorList = getAttributors();
//
//	if (attributorList.find(classname) != attributorList.end())
//	{
//		printf("Entity Attributor Factory for class '%s' already exists!\n", classname.c_str());
//		return;
//	}
//
//	printf("Entity Factory for class '%s' registered\n", classname.c_str());
//	constructorList[classname] = constructor;
//	attributorList[classname] = attributor;
//}
//
//// Constructs the entity by using the special macro generated EntityNameConstruct function.
//Entity* Factory::constructEntity(const std::string &classname)
//{
//	auto& constructorList = getConstructors();
//
//	if (constructorList.find(classname) == constructorList.end())
//	{
//		conoutf(CON_WARN, "Entity Factory for class '%s' missing!", classname.c_str());
//		return constructorList.at(fallbackEntityType)();
//	}
//	else
//	{
//		conoutf("Constructed class: %s", classname.c_str());
//		return constructorList.at(classname)();
//	}
//}
//
//
//AttributeList_T Factory::attributes(const std::string &classname)
//{
//	auto& attributorList = getAttributors();
//
//	if (attributorList.find(classname) == attributorList.end())
//	{
//		conoutf(CON_WARN, "Entity Factory for class '%s' missing!", classname.c_str());
//		return attributorList.at(fallbackEntityType)();
//	}
//	else
//	{
//		return attributorList.at(classname)();
//	}
//}
