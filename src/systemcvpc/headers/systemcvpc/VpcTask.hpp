// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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
 */

#ifndef _INCLUDED_SYSTEMCVPC_VPCTASK_HPP
#define _INCLUDED_SYSTEMCVPC_VPCTASK_HPP

#include "Component.hpp"
#include "ScheduledTask.hpp"

#include <CoSupport/SmartPtr/RefCountObject.hpp>

#include <boost/noncopyable.hpp>

namespace SystemC_VPC {

class VpcTask
  : private boost::noncopyable
  , public CoSupport::SmartPtr::RefCountObject
{
  typedef VpcTask this_type;
public:
  typedef boost::intrusive_ptr<this_type>       Ptr;
  typedef boost::intrusive_ptr<this_type const> ConstPtr;

  void mapTo(Component::Ptr component);

  void   setPriority(size_t priority);

  size_t getPriority() const;

  ScheduledTask const *getActor() const;

  Component::Ptr getComponent() const;

  void setActorAsPSM(bool psm);

  bool isPSM();

private:
  friend bool hasTask(ScheduledTask const &task);
  friend Ptr  getTask(ScheduledTask const &task);
  friend Ptr  createTask(ScheduledTask const &task);
  friend Ptr  createTask(std::string   const &taskName);

  VpcTask(ScheduledTask const *actor);
  VpcTask(std::string   const &actorName);

  void setActor(ScheduledTask const *actor);

  ~VpcTask();

  // configured data
  ScheduledTask const *task;
  std::string const    taskName;
  Component::Ptr       boundComponent;
  size_t               priority;
  bool                 psm;
};

bool hasTask(ScheduledTask const &task);
bool hasTask(std::string   const &taskName);

VpcTask::Ptr getTask(ScheduledTask const &task);
VpcTask::Ptr getTask(std::string   const &taskName);

VpcTask::Ptr createTask(ScheduledTask const &task);
VpcTask::Ptr createTask(std::string   const &taskName);

void registerComponentWakeup(const char *actor, VPCEvent::Ptr event);

void registerComponentIdle(const char *actor, VPCEvent::Ptr event);

/*
 * Sets the actor as a PSM actor.
 * The executing state of this actor will always set the component executing state to IDLE
 */
void setActorAsPSM(const char *actor, bool psm);

void changePowerMode(ScheduledTask &actor,std::string powermode);

bool hasWaitingOrRunningTasks(ScheduledTask &actor);

void setCanExec(ScheduledTask &actor, bool canExec);

void setPriority(ScheduledTask &actor, size_t priority);

ComponentInterface *getTaskComponentInterface(ScheduledTask &actor);

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_VPCTASK_HPP */
