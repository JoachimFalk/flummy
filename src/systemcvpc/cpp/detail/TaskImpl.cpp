// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2012 Tim Grocki <grocki@codesign.informatik.uni-erlangen.de>
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

#include <systemcvpc/TimingModifier.hpp>
#include <systemcvpc/vpc_config.h>

#include "AbstractComponent.hpp"
#include "TaskImpl.hpp"
#include "Director.hpp"

#include <CoSupport/Tracing/TracingFactory.hpp>

#include <ctime>
#include <cfloat>

#include "DebugOStream.hpp"

namespace SystemC_VPC { namespace Detail {

  TaskImpl::TaskImpl(TaskInterface *taskInterface)
    : Extending::Task(taskInterface->name())
    , scheduledTask(taskInterface)
    , pid(Director::getProcessId(taskInterface->name()))
    , priority(0)
    , psm(false) {}

  TaskImpl::TaskImpl(std::string const &taskName)
    : Extending::Task(taskName)
    , scheduledTask(nullptr)
    , pid(Director::getProcessId(taskName))
    , priority(0)
    , psm(false) {}

  TaskImpl::~TaskImpl() {
  }

  sc_core::sc_time TaskImpl::getPeriod() const{
    return sc_core::sc_time(DBL_MAX, sc_core::SC_SEC);
  }

} } // namespace SystemC_VPC::Detail
