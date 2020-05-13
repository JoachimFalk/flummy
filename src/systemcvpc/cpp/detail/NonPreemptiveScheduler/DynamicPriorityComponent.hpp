// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_NONPREEMPTIVESCHEDULER_DYNAMICPRIORITYCOMPONENT_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_NONPREEMPTIVESCHEDULER_DYNAMICPRIORITYCOMPONENT_HPP

#include "NonPreemptiveComponent.hpp"

#include <systemcvpc/Component.hpp>
#include <systemcvpc/VpcApi.hpp>

#include "../diagnostics/DebugOutput.hpp"
#include "../TaskInstanceImpl.hpp"

#include "config.h"
#include <systemcvpc/ScheduledTask.hpp>


#include <list>

namespace SystemC_VPC { namespace Detail {

typedef std::list<TaskInterface *> PriorityList;

class DynamicPriorityComponent: public NonPreemptiveComponent {
public:
  DynamicPriorityComponent(std::string const &name);

  /// Realize debug file interface from SystemC_VPC::Component.
  bool        hasDebugFile() const;
  /// Realize debug file interface from SystemC_VPC::Component.
  void        setDebugFileName(std::string const &fileName);
  /// Realize debug file interface from SystemC_VPC::Component.
  std::string getDebugFileName() const;

  // Implement ComponentInterface
  void                       setDynamicPriority(std::list<ScheduledTask *> priorityList);
  // Implement ComponentInterface
  std::list<ScheduledTask *> getDynamicPriority();

  // Implement ComponentInterface
  void scheduleAfterTransition();

  ~DynamicPriorityComponent();
protected:
  // Implement interface for NonPreemptiveComponent
  void newReadyTask(TaskInstanceImpl *newTask);

  // Implement interface for NonPreemptiveComponent
  TaskInstanceImpl *selectReadyTask();

  void start_of_simulation();

private:
  PriorityList   priorities_;

  std::list<TaskInstanceImpl *> readyTasks;

  TaskInterface *yieldTask;
  TaskInstanceImpl  *selectedTask;
  std::ostream  *debugOut;
  std::string    debugFileName;

  void debugDump(const TaskInterface * toBeExecuted) const;
};

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_NONPREEMPTIVESCHEDULER_DYNAMICPRIORITYCOMPONENT_HPP */
