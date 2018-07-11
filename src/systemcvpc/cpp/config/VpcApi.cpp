// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#include "config.h"

#include <systemcvpc/config/Component.hpp>
#include <systemcvpc/config/ConfigException.hpp>
#include <systemcvpc/config/VpcApi.hpp>
#include <systemcvpc/config/Mappings.hpp>

#include "../Director.hpp"

#include <string>
#include <iostream>

namespace SystemC_VPC { namespace Config {

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

static std::map<const TaskInterface *, VpcTask::Ptr> vpcTasks;

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
Component::Ptr createComponent(std::string name, Scheduler scheduler)
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
  ProcessId routePid = Detail::Director::getProcessId(source, dest);
  Route::Ptr route(new Route(type, source, dest));
  Routing::add(routePid, route);
  return route;
}

//
Route::Ptr createRoute(const sc_core::sc_port_base * leafPort, Route::Type type)
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
  ProcessId routePid = Detail::Director::getProcessId(source, dest);
  if (Routing::has(routePid)){
    return Routing::get(routePid);
  }

  throw ConfigException(std::string("Cannot get route \"") + source
      + " -> " + dest + "\" before creation. Use createRoute() first. ");
}

Route::Ptr getRoute(const sc_core::sc_port_base * leafPort){
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
  Detail::Director::getInstance().FALLBACKMODE = false;
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
  Detail::Director::getInstance().defaultRoute = ignore;
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
	if(Detail::Director::getInstance().FALLBACKMODE)
	{
		return;
	}
	Component::Ptr component = Mappings::getConfiguredMappings()[getCachedTask(actor)];
	if(component != NULL)
	{
		ComponentInterface* ci = component->getComponentInterface();
		if(ci != NULL)
			ci->changePowerMode(powermode);
	}


	//return NULL;
}

bool hasWaitingOrRunningTasks(ScheduledTask & actor){
  if(Detail::Director::getInstance().FALLBACKMODE)
   {
    //FIXME: how to handle Fallbackmode?
           return true;
   }
   Component::Ptr component = Mappings::getConfiguredMappings()[getCachedTask(actor)];
   if(component != NULL)
   {
           ComponentInterface* ci = component->getComponentInterface();
           if(ci != NULL)
                 return ci->hasWaitingOrRunningTasks();
   }
   //should never reach here
   return false;
}

void registerComponentWakeup(const char* actor, Coupling::VPCEvent::Ptr  event){
  if(Detail::Director::getInstance().FALLBACKMODE)
  {
          return;
  }
  Component::Ptr component = Mappings::getConfiguredMappings()[getCachedTask(actor)];
  if(component != NULL)
  {
          ComponentInterface* ci = component->getComponentInterface();
          if(ci != NULL)
                  ci->registerComponentWakeup(getCachedTask(actor)->getActor(), event);
  }
}

void registerComponentIdle(const char* actor, Coupling::VPCEvent::Ptr  event){

  if(Detail::Director::getInstance().FALLBACKMODE)
  {
          return;
  }
  Component::Ptr component = Mappings::getConfiguredMappings()[getCachedTask(actor)];
  if(component != NULL)
  {
          ComponentInterface* ci = component->getComponentInterface();
          if(ci != NULL)
                  ci->registerComponentIdle(getCachedTask(actor)->getActor(), event);
  }
}

void setCanExec(ScheduledTask & actor, bool canExec){
  if(Detail::Director::getInstance().FALLBACKMODE){
            return;
    }
    Component::Ptr component = Mappings::getConfiguredMappings()[getCachedTask(actor)];
    if(component != NULL)
    {
            ComponentInterface* ci = component->getComponentInterface();
            if(ci != NULL)
                    ci->setCanExec(canExec);
    }
}

void setActorAsPSM(const char* name, bool psm)
{
	  getCachedTask(name)->setActorAsPSM(psm);
}

} } // namespace SystemC_VPC::Config
