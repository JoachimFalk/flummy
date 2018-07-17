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
  typedef boost::intrusive_ptr<this_type> const ConstPtr;

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
