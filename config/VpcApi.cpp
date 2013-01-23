/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 *
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#include <systemcvpc/config/Component.hpp>
#include <systemcvpc/config/ConfigException.hpp>
#include <systemcvpc/config/VpcApi.hpp>
#include <systemcvpc/Director.hpp>

#include "Mappings.hpp"

#include <string>
#include <iostream>

namespace SystemC_VPC
{
namespace Config
{

Modifiers & getDistributions()
{
	static Modifiers modifiers;
	return modifiers;
}

//
Components & getComponents()
{
  static Components components;
  return components;
}

//
std::map<std::string, VpcTask::Ptr>& getVpcTasksByName()
{
  static std::map<std::string, VpcTask::Ptr> vpcTasksByName;
  return vpcTasksByName;
}

static std::map<const ScheduledTask *, VpcTask::Ptr> vpcTasks;

void createDistribution(std::string name, boost::shared_ptr<DistributionTimingModifier> modifier)
{
	if(!hasDistribution(name)) {
		getDistributions()[name] = modifier;
		return;
	}
  throw ConfigException(std::string("Multiple creation of distribution \"") + name
      + "\" is not supported. Use getDistribution() instead. ");
}

//
Component::Ptr createComponent(std::string name, Scheduler::Type scheduler)
{
  if (!hasComponent(name)) {
    Component::Ptr ptr(new Component(name, scheduler));

    getComponents()[name] = ptr;
    return ptr;
  }
  throw ConfigException(std::string("Multiple creation of component \"") + name
      + "\" is not supported. Use getComponent() instead. ");
}

//
Component::Ptr getComponent(std::string name)
{
  if (hasComponent(name)) {
    return getComponents()[name];
  }
  throw ConfigException(std::string("Cannot get component \"") + name
      + "\" before creation. Use createComponent() first. ");
}

//
Route::Ptr createRoute(std::string source, std::string dest, Route::Type type)
{
  ProcessId routePid = Director::getProcessId(source, dest);
  Route::Ptr route(new Route(type, source, dest));
  Routing::add(routePid, route);
  return route;
}

//
Route::Ptr createRoute(const sc_port_base * leafPort, Route::Type type)
{
  // TODO: ProcessId routePid = Director::getProcessId(source, dest);
  assert(leafPort != NULL);
  Route::Ptr route(new Route(type));
  Routing::add(leafPort, route);
  return route;
}

//
Route::Ptr getRoute(std::string source, std::string dest)
{
  ProcessId routePid = Director::getProcessId(source, dest);
  if (Routing::has(routePid)){
    return Routing::get(routePid);
  }

  throw ConfigException(std::string("Cannot get route \"") + source
      + " -> " + dest + "\" before creation. Use createRoute() first. ");
}

Route::Ptr getRoute(const sc_port_base * leafPort){
  return  Routing::get(leafPort);
}

bool hasDistribution(std::string name)
{
	return getDistributions().find(name) != getDistributions().end();
}

//
bool hasComponent(std::string name)
{
  return getComponents().find(name) != getComponents().end();
}

//
VpcTask::Ptr getCachedTask(ScheduledTask & actor)
{
  // TODO: find the right place
  SystemC_VPC::Director::getInstance().FALLBACKMODE = false;
  //std::cerr << " unset FALLBACKMODE" << std::endl;


  // the pid is not yet injected
  //std::cerr << "createTask: " << actor.getPid() << std::endl;
  if (!hasTask(actor)) {
    vpcTasks[&actor] = VpcTask::Ptr(new VpcTask(actor));
  }
  return vpcTasks[&actor];
}

//
VpcTask::Ptr getCachedTask(std::string name)
{
  if (!hasTask(name)) {
    getVpcTasksByName()[name] = VpcTask::Ptr(new VpcTask());
  }
  return getVpcTasksByName()[name];
}

//
void setCachedTask(const ScheduledTask * actor, VpcTask::Ptr task)
{
  assert( !hasTask(*actor) );
  vpcTasks[actor] = task;
}

//
void setCachedTask(std::string name, VpcTask::Ptr task)
{
  assert( !hasTask(name) );
  getVpcTasksByName()[name] = task;
}

//
bool hasTask(const ScheduledTask & actor)
{
  return vpcTasks.find(&actor) != vpcTasks.end();
}

//
bool hasTask(std::string name)
{
  return getVpcTasksByName().find(name) != getVpcTasksByName().end();
}

//
void setPriority(ScheduledTask & actor, size_t priority)
{
  getCachedTask(actor)->setPriority(priority);
}

//
void ignoreMissingRoutes(bool ignore)
{
  Director::getInstance().defaultRoute = ignore;
}

ComponentInterface* getTaskComponentInterface(ScheduledTask & actor)
{
	Component::Ptr component = Mappings::getConfiguredMappings()[getCachedTask(actor)];
	if(component != NULL)
		return component->getComponentInterface();
	else
		return NULL;

	//return NULL;
}

void changePowerMode(ScheduledTask & actor,std::string powermode)
{
	if(Director::getInstance().FALLBACKMODE)
	{
		return;
	}
	Component::Ptr component = Mappings::getConfiguredMappings()[getCachedTask(actor)];
	if(component != NULL)
	{
		SystemC_VPC::ComponentInterface* ci = component->getComponentInterface();
		if(ci != NULL)
			ci->changePowerMode(powermode);
	}


	//return NULL;
}

bool hasWaitingOrRunningTasks(ScheduledTask & actor){
  if(Director::getInstance().FALLBACKMODE)
   {
    //FIXME: how to handle Fallbackmode?
           return true;
   }
   Component::Ptr component = Mappings::getConfiguredMappings()[getCachedTask(actor)];
   if(component != NULL)
   {
           SystemC_VPC::ComponentInterface* ci = component->getComponentInterface();
           if(ci != NULL)
                 return ci->hasWaitingOrRunningTasks();
   }
   //should never reach here
   return false;
}

void registerComponentWakeup(const char* actor, Coupling::VPCEvent::Ptr  event){
  if(Director::getInstance().FALLBACKMODE)
  {
          return;
  }
  Component::Ptr component = Mappings::getConfiguredMappings()[getCachedTask(actor)];
  if(component != NULL)
  {
          SystemC_VPC::ComponentInterface* ci = component->getComponentInterface();
          if(ci != NULL)
                  ci->registerComponentWakeup(getCachedTask(actor)->getActor(), event);
  }
}

void registerComponentIdle(const char* actor, Coupling::VPCEvent::Ptr  event){

  if(Director::getInstance().FALLBACKMODE)
  {
          return;
  }
  Component::Ptr component = Mappings::getConfiguredMappings()[getCachedTask(actor)];
  if(component != NULL)
  {
          SystemC_VPC::ComponentInterface* ci = component->getComponentInterface();
          if(ci != NULL)
                  ci->registerComponentIdle(getCachedTask(actor)->getActor(), event);
  }
}

void setCanExec(ScheduledTask & actor, bool canExec){
  if(Director::getInstance().FALLBACKMODE){
            return;
    }
    Component::Ptr component = Mappings::getConfiguredMappings()[getCachedTask(actor)];
    if(component != NULL)
    {
            SystemC_VPC::ComponentInterface* ci = component->getComponentInterface();
            if(ci != NULL)
                    ci->setCanExec(canExec);
    }
}

void setActorAsPSM(const char* name, bool psm)
{
	  getCachedTask(name)->setActorAsPSM(psm);
}

} // namespace Config
} // namespace SystemC_VPC
