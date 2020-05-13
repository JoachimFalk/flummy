// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_EXTENDING_TASKINSTANCE_HPP
#define _INCLUDED_SYSTEMCVPC_EXTENDING_TASKINSTANCE_HPP

#include "../Power.hpp"

#include <systemc>

#include <boost/noncopyable.hpp>

namespace SystemC_VPC { namespace Extending {

  class Task;

  /**
   * This class represents the publicly visible information of a
   * task instance, i.e., one execution of a task on a component.
   */
  class TaskInstance: private boost::noncopyable {
  public:

    enum class Type {
      ACTION, GUARD, MESSAGE
    };

    // Getters
    Task            *getTask() const
      { return task; }
    Type             getType() const
      { return type; }
    std::string      getName() const;

    sc_core::sc_time getDelay() const
      { return this->delay; }
    sc_core::sc_time getLatency() const
      { return this->latency; }
    sc_core::sc_time getRemainingDelay() const
      { return this->remainingDelay; }

    Power            getPower() const
      { return pwr; }

  protected:
    TaskInstance(Task *task, Type type)
      : task(task), type(type) {}

    void setDelay(sc_core::sc_time delay)
      { this->delay = delay; }
    void setLatency(sc_core::sc_time latency)
      { this->latency = latency; }
    void setRemainingDelay(sc_core::sc_time delay)
      { this->remainingDelay = delay; }

    void setPower(Power pwr)
      { this->pwr = pwr; }
  private:
    Task            *task;
    Type             type; /// Type of task instance, i.e., action, guard, or message.

    // Timings
    sc_core::sc_time delay;
    sc_core::sc_time latency;
    sc_core::sc_time remainingDelay;

    Power pwr; ///< Power consumption when this task instance is running on a component.
  };

} } // namespace SystemC_VPC::Extending

#endif /* _INCLUDED_SYSTEMCVPC_EXTENDING_TASKINSTANCE_HPP */
