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
  Route::Ptr route(new Route(source, dest, type));
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

//
bool hasComponent(std::string name)
{
  return getComponents().find(name) != getComponents().end();
}

//
VpcTask::Ptr getCachedTask(const ScheduledTask & actor)
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
void setPriority(const ScheduledTask & actor, size_t priority)
{
  getCachedTask(actor)->setPriority(priority);
}

//
void ignoreMissingRoutes(bool ignore)
{
  Director::getInstance().defaultRoute = ignore;
}
} // namespace Config
} // namespace SystemC_VPC
