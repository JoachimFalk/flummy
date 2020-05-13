// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_NONPREEMPTIVESCHEDULER_ROUNDROBINCOMPONENT_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_NONPREEMPTIVESCHEDULER_ROUNDROBINCOMPONENT_HPP

#include "../AbstractComponent.hpp"

//#include "memory.h"

#include <vector>
#include <deque>

namespace SystemC_VPC { namespace Detail {

class RoundRobinComponent : public AbstractComponent {
  SC_HAS_PROCESS(RoundRobinComponent);

  typedef AbstractComponent base_type;
public:
  RoundRobinComponent(std::string const &name);

protected:
  void setActivationCallback(bool flag);

  void end_of_elaboration();

  void notifyActivation(TaskInterface * scheduledTask, bool active);

  void compute(TaskInstanceImpl *actualTask);

  void check(TaskInstanceImpl *actualTask);

  /**
   *
   */
  void requestBlockingCompute(TaskInstanceImpl* task, VPCEvent::Ptr blocker);

  /**
   *
   */
  void execBlockingCompute(TaskInstanceImpl* task, VPCEvent::Ptr blocker);

  /**
   *
   */
  void abortBlockingCompute(TaskInstanceImpl* task, VPCEvent::Ptr blocker);

  /*
   * from ComponentInterface
   */
  bool hasWaitingOrRunningTasks();

  bool scheduleMessageTasks();

  void scheduleThread();

  void addAttribute(Attribute const &attribute);

  virtual ~RoundRobinComponent();
private:
  bool useActivationCallback;

  /// Indicates if this scheduler will fire an actor till it is no longer fireable.
  bool fireActorInLoop;

  /// This list contains the message tasks that will appear
  /// via compute calls.
  std::deque<TaskInstanceImpl *>    readyMsgTasks;
  /// This list represent all the SysteMoC actors that
  /// are mapped to this component. The list will be
  /// filled by the the end_of_elaboration method.
  std::vector<TaskInterface *>  taskList;
  /// This is the actual running task that will
  /// be assigned by the compute method if
  /// on of the SysteMoC actors of the component
  /// is currently running.
  TaskInstanceImpl                 *actualTask;
  sc_core::sc_event             readyEvent;
};

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_NONPREEMPTIVESCHEDULER_ROUNDROBINCOMPONENT_HPP */
