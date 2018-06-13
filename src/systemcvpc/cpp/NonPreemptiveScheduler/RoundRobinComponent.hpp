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

#ifndef _INCLUDED_SYSTEMCVPC_NONPREEMPTIVESCHEDULER_ROUNDROBINCOMPONENT_HPP
#define _INCLUDED_SYSTEMCVPC_NONPREEMPTIVESCHEDULER_ROUNDROBINCOMPONENT_HPP

#include "memory.h"

#include <vector>
#include <deque>

#include "../AbstractComponent.hpp"

#include <systemcvpc/Director.hpp>

namespace SystemC_VPC{

class RoundRobinComponent : public AbstractComponent {
  SC_HAS_PROCESS(RoundRobinComponent);
public:
  RoundRobinComponent(Config::Component::Ptr component,
      Director *director = &Director::getInstance());

protected:
  bool useActivationCallback;

  void setActivationCallback(bool flag);

  void end_of_elaboration();

  void notifyActivation(TaskInterface * scheduledTask, bool active);

  void compute(TaskInstance *actualTask);

  void check(TaskInstance *actualTask);

  /**
   *
   */
  virtual void requestBlockingCompute(TaskInstance* task, Coupling::VPCEvent::Ptr blocker);

  /**
   *
   */
  virtual void execBlockingCompute(TaskInstance* task, Coupling::VPCEvent::Ptr blocker);

  /**
   *
   */
  virtual void abortBlockingCompute(TaskInstance* task, Coupling::VPCEvent::Ptr blocker);

  /**
   *
   */
  virtual void updatePowerConsumption();

  /*
   * from ComponentInterface
   */
  bool hasWaitingOrRunningTasks();

  bool scheduleMessageTasks();

  void scheduleThread();

  virtual ~RoundRobinComponent();
private:

  /// This list contains the message tasks that will appear
  /// via compute calls.
  std::deque<TaskInstance *>    readyMsgTasks;
  /// This list represent all the SysteMoC actors that
  /// are mapped to this component. The list will be
  /// filled by the the end_of_elaboration method.
  std::vector<TaskInterface *>  taskList;
  /// This is the actual running task that will
  /// be assigned by the compute method if
  /// on of the SysteMoC actors of the component
  /// is currently running.
  TaskInstance                 *actualTask;
  sc_core::sc_event             readyEvent;
};

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_NONPREEMPTIVESCHEDULER_ROUNDROBINCOMPONENT_HPP */
