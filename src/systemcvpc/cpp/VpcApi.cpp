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

#include <systemcvpc/Component.hpp>
#include <systemcvpc/ConfigException.hpp>
#include <systemcvpc/VpcApi.hpp>
#include <systemcvpc/Mappings.hpp>
#include <systemcvpc/Scheduler.hpp>

#include "detail/NonPreemptiveScheduler/DynamicPriorityComponent.hpp"
#include "detail/NonPreemptiveScheduler/FcfsComponent.hpp"
#include "detail/NonPreemptiveScheduler/NonPreemptiveComponent.hpp"
#include "detail/NonPreemptiveScheduler/PriorityComponent.hpp"
#include "detail/NonPreemptiveScheduler/RoundRobinComponent.hpp"
#include "detail/PreemptiveScheduler/PreemptiveComponent.hpp"
#include "detail/PreemptiveScheduler/AVBScheduler.hpp"
#include "detail/PreemptiveScheduler/FlexRayScheduler.hpp"
#include "detail/PreemptiveScheduler/MostScheduler.hpp"
#include "detail/PreemptiveScheduler/MostSecondaryScheduler.hpp"
#include "detail/PreemptiveScheduler/PriorityScheduler.hpp"
#include "detail/PreemptiveScheduler/RateMonotonicScheduler.hpp"
#include "detail/PreemptiveScheduler/RoundRobinScheduler.hpp"
#include "detail/PreemptiveScheduler/StreamShaperScheduler.hpp"
#include "detail/PreemptiveScheduler/TDMAScheduler.hpp"
#include "detail/PreemptiveScheduler/TimeTriggeredCCScheduler.hpp"
#include "detail/Director.hpp"

#include <string>
#include <iostream>

namespace SystemC_VPC {

Components &getComponents() {
  static Components components;
  return components;
}

bool hasComponent(std::string name) {
  return getComponents().find(name) != getComponents().end();
}

Component::Ptr createComponent(std::string name, Scheduler scheduler) {
  std::pair<Components::iterator, bool> status =
      getComponents().insert(Components::value_type(name, nullptr));
  if (!status.second)
    throw ConfigException(std::string("Multiple creation of component \"") + name
        + "\" is not supported. Use getComponent() instead.");

  Component::Ptr &comp = status.first->second;

  switch (scheduler) {
    case Scheduler::FCFS:
      comp = new Detail::FcfsComponent(name);
      break;
    case Scheduler::StaticPriorityNoPreempt:
      comp = new Detail::PriorityComponent(name);
      break;
    case Scheduler::RoundRobinNoPreempt:
      comp = new Detail::RoundRobinComponent(name);
      break;
    case Scheduler::DynamicPriorityUserYield:
      comp = new Detail::DynamicPriorityComponent(name);
      break;
    case Scheduler::RoundRobin:
      comp = new Detail::PreemptiveComponent(name, new Detail::RoundRobinScheduler());
      break;
    case Scheduler::StaticPriority:
      comp = new Detail::PreemptiveComponent(name, new Detail::PriorityScheduler());
      break;
    case Scheduler::RateMonotonic:
      comp = new Detail::PreemptiveComponent(name, new Detail::RateMonotonicScheduler());
      break;
    case Scheduler::TDMA:
      comp = new Detail::PreemptiveComponent(name, new Detail::TDMAScheduler());
      break;
    case Scheduler::FlexRay:
      comp = new Detail::PreemptiveComponent(name, new Detail::FlexRayScheduler());
      break;
    case Scheduler::AVB:
      comp = new Detail::PreemptiveComponent(name, new Detail::AVBScheduler());
      break;
    case Scheduler::TTCC:
      comp = new Detail::PreemptiveComponent(name, new Detail::TimeTriggeredCCScheduler());
      break;
    case Scheduler::MOST:
      comp = new Detail::PreemptiveComponent(name, new Detail::MostScheduler());
      break;
    case Scheduler::StreamShaper:
      comp = new Detail::PreemptiveComponent(name, new Detail::StreamShaperScheduler());
      break;
    default:
      assert(!"Oops, I don't know this scheduler!");
  }
  return comp;
}

Component::Ptr getComponent(std::string name) {
  Components::iterator iter = getComponents().find(name);
  if (iter == getComponents().end())
    throw ConfigException(std::string("Cannot get component \"") + name
        + "\" before creation. Use createComponent() first.");
  return iter->second;
}


Route::Ptr createRoute(std::string source, std::string dest, Route::Type type) {
  ProcessId routePid = Detail::Director::getProcessId(source, dest);
  Route::Ptr route(new Route(type, source, dest));
  Routing::add(routePid, route);
  return route;
}

Route::Ptr createRoute(const sc_core::sc_port_base * leafPort, Route::Type type) {
  // TODO: ProcessId routePid = Director::getProcessId(source, dest);
  assert(leafPort != NULL);
  Route::Ptr route(new Route(type));
  Routing::add(leafPort, route);
  return route;
}

Route::Ptr getRoute(std::string source, std::string dest) {
  ProcessId routePid = Detail::Director::getProcessId(source, dest);
  if (Routing::has(routePid)){
    return Routing::get(routePid);
  }

  throw ConfigException(std::string("Cannot get route \"") + source
      + " -> " + dest + "\" before creation. Use createRoute() first.");
}

Route::Ptr getRoute(const sc_core::sc_port_base * leafPort){
  return  Routing::get(leafPort);
}

typedef std::map<std::string, VpcTask::Ptr> VpcTasks;

static
VpcTasks &getVpcTasks() {
  static VpcTasks vpcTasksByName;
  return vpcTasksByName;
}

bool hasTask(ScheduledTask const &task) {
  VpcTasks::iterator iter = getVpcTasks().find(task.name());
  if (iter == getVpcTasks().end())
    return false;
  iter->second->setActor(&task);
  return true;
}

bool hasTask(std::string   const &taskName) {
  return getVpcTasks().find(taskName) != getVpcTasks().end();
}

VpcTask::Ptr createTask(ScheduledTask const &task) {
  // TODO: find the right place
  Detail::Director::getInstance().FALLBACKMODE = false;

  std::pair<VpcTasks::iterator, bool> status =
      getVpcTasks().insert(VpcTasks::value_type(task.name(), nullptr));
  if (!status.second)
    throw ConfigException(std::string("Multiple creation of task \"") + task.name()
        + "\" is not supported. Use getTask() instead.");
  status.first->second = new VpcTask(&task);
  return status.first->second;
}

VpcTask::Ptr createTask(std::string   const &taskName) {
  // TODO: find the right place
  Detail::Director::getInstance().FALLBACKMODE = false;

  std::pair<VpcTasks::iterator, bool> status =
      getVpcTasks().insert(VpcTasks::value_type(taskName, nullptr));
  if (!status.second)
    throw ConfigException(std::string("Multiple creation of task \"") + taskName
        + "\" is not supported. Use getTask() instead.");
  status.first->second = new VpcTask(taskName);
  return status.first->second;
}

VpcTask::Ptr getTask(ScheduledTask const &task) {
  VpcTasks::iterator iter = getVpcTasks().find(task.name());
  if (iter == getVpcTasks().end())
    throw ConfigException(std::string("Cannot get task \"") + task.name()
        + "\" before creation. Use createTask() first.");
  iter->second->setActor(&task);
  return iter->second;
}

VpcTask::Ptr getTask(std::string   const &taskName) {
  VpcTasks::iterator iter = getVpcTasks().find(taskName);
  if (iter == getVpcTasks().end())
    throw ConfigException(std::string("Cannot get task \"") + taskName
        + "\" before creation. Use createTask() first.");
  return iter->second;
}

Modifiers & getDistributions()
{
  static Modifiers modifiers;
  return modifiers;
}


void createDistribution(std::string name, boost::shared_ptr<DistributionTimingModifier> modifier)
{
  if(!hasDistribution(name)) {
    getDistributions()[name] = modifier;
    return;
  }
  throw ConfigException(std::string("Multiple creation of distribution \"") + name
      + "\" is not supported. Use getDistribution() instead. ");
}


bool hasDistribution(std::string name)
{
  return getDistributions().find(name) != getDistributions().end();
}

void setPriority(ScheduledTask & actor, size_t priority)
{
  getTask(actor)->setPriority(priority);
}

//
void ignoreMissingRoutes(bool ignore)
{
  Detail::Director::getInstance().defaultRoute = ignore;
}

ComponentInterface* getTaskComponentInterface(ScheduledTask & actor)
{
  Component::Ptr component = Mappings::getConfiguredMappings()[getTask(actor)];
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
  Component::Ptr component = Mappings::getConfiguredMappings()[getTask(actor)];
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
   Component::Ptr component = Mappings::getConfiguredMappings()[getTask(actor)];
   if(component != NULL)
   {
     ComponentInterface* ci = component->getComponentInterface();
     if(ci != NULL)
       return ci->hasWaitingOrRunningTasks();
   }
   //should never reach here
   return false;
}

void registerComponentWakeup(const char* actor, VPCEvent::Ptr  event){
  if(Detail::Director::getInstance().FALLBACKMODE)
  {
    return;
  }
  Component::Ptr component = Mappings::getConfiguredMappings()[getTask(actor)];
  if(component != NULL)
  {
    ComponentInterface* ci = component->getComponentInterface();
    if(ci != NULL)
      ci->registerComponentWakeup(getTask(actor)->getActor(), event);
  }
}

void registerComponentIdle(const char* actor, VPCEvent::Ptr  event){

  if(Detail::Director::getInstance().FALLBACKMODE)
  {
    return;
  }
  Component::Ptr component = Mappings::getConfiguredMappings()[getTask(actor)];
  if(component != NULL)
  {
    ComponentInterface* ci = component->getComponentInterface();
    if(ci != NULL)
      ci->registerComponentIdle(getTask(actor)->getActor(), event);
  }
}

void setCanExec(ScheduledTask & actor, bool canExec){
  if(Detail::Director::getInstance().FALLBACKMODE){
    return;
  }
  Component::Ptr component = Mappings::getConfiguredMappings()[getTask(actor)];
  if(component != NULL)
  {
    ComponentInterface* ci = component->getComponentInterface();
    if(ci != NULL)
      ci->setCanExec(canExec);
  }
}

void setActorAsPSM(const char* name, bool psm)
{
  getTask(name)->setActorAsPSM(psm);
}

} // namespace SystemC_VPC
