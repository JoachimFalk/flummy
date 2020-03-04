// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2020 Hardware-Software-CoDesign, University of
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_TIMEQUEUE_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_TIMEQUEUE_HPP

#include "TaskInstanceImpl.hpp"

#include <systemc>

#include <queue>
#include <functional>

namespace SystemC_VPC { namespace Detail {

  class TimeQueue
    : public sc_core::sc_module
  {
    SC_HAS_PROCESS(TimeQueue);
  public:
    TimeQueue(sc_core::sc_module_name name, std::function<void (TaskInstanceImpl *)> const &callback);

    void add(TaskInstanceImpl *task, sc_core::sc_time delay);
  private:
    struct QueueEntry {
      QueueEntry(sc_core::sc_time time, TaskInstanceImpl *ti)
        : time(time), ti(ti) {}

      bool operator<(QueueEntry const &right) const
        { return time > right.time; }

      sc_core::sc_time time;
      TaskInstanceImpl *ti;
    };

    typedef std::priority_queue<QueueEntry> Queue;

    void queueMethod();

    std::function<void (TaskInstanceImpl *)> callback;

    Queue             queue;
    sc_core::sc_event queueEvent;
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_TIMEQUEUE_HPP */
