// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
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

#include "TaskInstanceImpl.hpp"
#include "TaskImpl.hpp"

namespace SystemC_VPC { namespace Detail {

  int TaskInstanceImpl::globalInstanceId = 0;

  TaskInstanceImpl::TaskInstanceImpl(
      TaskImpl                                       *taskImpl
    , Type                                            type
    , PossibleAction                                 *firingRuleInterface
    , std::function<void (TaskInstanceImpl *)> const &diiCallback
    , std::function<void (TaskInstanceImpl *)> const &latCallback)
    : base_type(taskImpl, type)
    , instanceId(globalInstanceId++)
    , firingRuleInterface(firingRuleInterface)
    , diiCallback(diiCallback)
    , latCallback(latCallback)
  {}

  TaskInstanceImpl::~TaskInstanceImpl() {
  }

  // Adaptor getter for TaskImpl
  int              TaskInstanceImpl::getPriority() const
    { assert(getTask() != NULL); return getTask()->getPriority(); }
  sc_core::sc_time TaskInstanceImpl::getPeriod() const
    { assert(getTask() != NULL); return getTask()->getPeriod(); }
  ProcessId        TaskInstanceImpl::getProcessId() const
    { assert(getTask() != NULL); return getTask()->getPid(); }
  bool             TaskInstanceImpl::isPSM() const
    { assert(getTask() != NULL); return getTask()->getTaskIsPSM(); }

} } // namespace SystemC_VPC::Detail
