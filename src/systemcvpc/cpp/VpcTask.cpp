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

#include <systemcvpc/VpcTask.hpp>
#include <systemcvpc/Component.hpp>
#include <systemcvpc/ConfigException.hpp>
#include <systemcvpc/ScheduledTask.hpp>

#include "detail/Configuration.hpp"

namespace SystemC_VPC {

  VpcTask::VpcTask(ScheduledTask const *actor)
    : task(actor)
    , taskName(actor->name())
    , priority(0)
    , psm(false) {}

  VpcTask::VpcTask(std::string   const &actorName)
    : task(nullptr) // Later, must be set by setActor
    , taskName(actorName)
    , priority(0)
    , psm(false) {}

  VpcTask::~VpcTask() {}

  void VpcTask::mapTo(Component::Ptr component) {
    assert(component && "Only a valid component can be mapped to!");
    assert((!boundComponent || boundComponent == component) && "Binding the actor to a component can be done only once!");
    boundComponent = component;
  }

  void VpcTask::setPriority(size_t priority) {
    this->priority = priority;
  }

  size_t VpcTask::getPriority() const {
    return this->priority;
  }

  const ScheduledTask *VpcTask::getActor() const {
    return this->task;
  }

  Component::Ptr VpcTask::getComponent() const {
    return this->boundComponent;
  }

  void VpcTask::setActor(ScheduledTask const *actor) {
    assert(actor && actor->name() == taskName && "Only a valid actor can be set!");
    assert((!task || task == actor) && "Setting the actor can be done only once!");
    task = actor;
  }

  void VpcTask::setActorAsPSM(bool psm) {
    this->psm = psm;
  }

  bool VpcTask::isPSM() {
    return this->psm;
  }

  bool hasTask(ScheduledTask const &task) {
    VpcTask::Ptr vpcTask = Detail::Configuration::getInstance().hasVpcTask(task.name());
    if (!vpcTask.get())
      return false;
    vpcTask->setActor(&task);
    return true;
  }

  bool hasTask(std::string   const &taskName) {
    return Detail::Configuration::getInstance().hasVpcTask(taskName).get();
  }

  VpcTask::Ptr createTask(ScheduledTask const &task) {
    return Detail::Configuration::getInstance().createVpcTask(task.name(),
        [&task]() { return new VpcTask(&task); });
  }

  VpcTask::Ptr createTask(std::string   const &taskName) {
    return Detail::Configuration::getInstance().createVpcTask(taskName,
        [&taskName]() { return new VpcTask(taskName); });
  }

  VpcTask::Ptr getTask(ScheduledTask const &task) {
    VpcTask::Ptr vpcTask = Detail::Configuration::getInstance().getVpcTask(task.name());
    vpcTask->setActor(&task);
    return vpcTask;
  }

  VpcTask::Ptr getTask(std::string   const &taskName) {
    return Detail::Configuration::getInstance().getVpcTask(taskName);
  }

  void registerComponentWakeup(const char *actor, VPCEvent::Ptr  event) {
//  if(Detail::Director::getInstance().FALLBACKMODE)
//  {
//    return;
//  }
    Component::Ptr component = getTask(actor)->getComponent();
    if(component != NULL)
    {
      ComponentInterface* ci = component->getComponentInterface();
      if(ci != NULL)
        ci->registerComponentWakeup(getTask(actor)->getActor(), event);
    }
  }

  void registerComponentIdle(const char *actor, VPCEvent::Ptr  event) {
//  if(Detail::Director::getInstance().FALLBACKMODE)
//  {
//    return;
//  }
    Component::Ptr component = getTask(actor)->getComponent();
    if(component != NULL)
    {
      ComponentInterface* ci = component->getComponentInterface();
      if(ci != NULL)
        ci->registerComponentIdle(getTask(actor)->getActor(), event);
    }
  }

  void setActorAsPSM(const char *actor, bool psm) {
    getTask(actor)->setActorAsPSM(psm);
  }

  void changePowerMode(ScheduledTask &actor, std::string powermode) {
//  if(Detail::Director::getInstance().FALLBACKMODE)
//  {
//    return;
//  }
    Component::Ptr component = getTask(actor)->getComponent();
    if(component != NULL)
    {
      ComponentInterface* ci = component->getComponentInterface();
      if(ci != NULL)
        ci->changePowerMode(powermode);
    }
    //return NULL;
  }

  bool hasWaitingOrRunningTasks(ScheduledTask & actor) {
//  if(Detail::Director::getInstance().FALLBACKMODE) {
//    //FIXME: how to handle Fallbackmode?
//    return true;
//  }
    Component::Ptr component = getTask(actor)->getComponent();
    if(component != NULL)
    {
      ComponentInterface* ci = component->getComponentInterface();
      if(ci != NULL)
        return ci->hasWaitingOrRunningTasks();
    }
    //should never reach here
    return false;
  }

  void setCanExec(ScheduledTask & actor, bool canExec) {
//  if(Detail::Director::getInstance().FALLBACKMODE){
//    return;
//  }
    Component::Ptr component = getTask(actor)->getComponent();
    if(component != NULL)
    {
      ComponentInterface* ci = component->getComponentInterface();
      if(ci != NULL)
        ci->setCanExec(canExec);
    }
  }

  void setPriority(ScheduledTask &actor, size_t priority) {
    getTask(actor)->setPriority(priority);
  }

  ComponentInterface *getTaskComponentInterface(ScheduledTask &actor) {
    Component::Ptr component = getTask(actor)->getComponent();
    if(component != NULL)
      return component->getComponentInterface();
    else
      return NULL;
  }

} // namespace SystemC_VPC
