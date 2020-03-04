// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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
