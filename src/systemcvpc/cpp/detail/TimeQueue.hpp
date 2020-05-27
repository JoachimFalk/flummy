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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_TIMEQUEUE_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_TIMEQUEUE_HPP

#include "TaskInstanceImpl.hpp"

#include <systemc>

#include <queue>
#include <functional>

namespace SystemC_VPC { namespace Detail {

  class TimeQueueImpl
    : public sc_core::sc_module
  {
    SC_HAS_PROCESS(TimeQueueImpl);
  protected:
    TimeQueueImpl(sc_core::sc_module_name const &name, std::function<void (void *)> const &callback);

    void add(void *obj, sc_core::sc_time delay);
  private:
    struct QueueEntry {
      QueueEntry(sc_core::sc_time time, void *obj)
        : time(time), obj(obj) {}

      bool operator<(QueueEntry const &right) const
        { return time > right.time; }

      sc_core::sc_time  time;
      void             *obj;
    };

    typedef std::priority_queue<QueueEntry> Queue;

    void queueMethod();

    std::function<void (void *)> callback;

    Queue             queue;
    sc_core::sc_event queueEvent;
  };

  template <class T>
  class TimeQueue
    : public TimeQueueImpl
  {
    typedef TimeQueueImpl base_type;
  public:
    TimeQueue(sc_core::sc_module_name name, std::function<void (T *)> const &callback)
      : TimeQueueImpl(name, reinterpret_cast<std::function<void (void *)> const &>(callback)) {}

    /// Add a object to the queue. After the given delay, the callback given
    /// in the constructor will be called with this object.
    void add(T *obj, sc_core::sc_time delay)
      { base_type::add(obj, delay); }
  };


} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_TIMEQUEUE_HPP */
