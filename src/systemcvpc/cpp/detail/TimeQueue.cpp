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

#include "TimeQueue.hpp"

namespace SystemC_VPC { namespace Detail {

  TimeQueue::TimeQueue(sc_core::sc_module_name name, std::function<void (TaskInstanceImpl *)> const &callback)
    : sc_core::sc_module(name), callback(callback)
  {
    SC_METHOD(queueMethod);
    dont_initialize();
    sensitive << queueEvent;
  }

  void TimeQueue::queueMethod() {
    assert(!queue.empty());
    while (true) {
      assert(queue.top().time == sc_core::sc_time_stamp());
      TaskInstanceImpl *ti = queue.top().ti;
      queue.pop();
      callback(ti);
      if (queue.empty())
        break;
      sc_core::sc_time delta = queue.top().time -
          sc_core::sc_time_stamp();
      if (delta > sc_core::SC_ZERO_TIME) {
        queueEvent.notify(delta);
        break;
      }
    }
  }

  void TimeQueue::add(TaskInstanceImpl *ti, sc_core::sc_time delay) {

    if (delay > sc_core::SC_ZERO_TIME) {
      queue.push(QueueEntry(sc_core::sc_time_stamp() + delay, ti));
      queueEvent.notify(delay);
    } else {
      assert(delay == sc_core::SC_ZERO_TIME);
      callback(ti);
    }
  }

} } // namespace SystemC_VPC::Detail
