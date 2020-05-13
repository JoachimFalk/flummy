// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_TIMETRIGGERED_TT_SUPPORT_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_TIMETRIGGERED_TT_SUPPORT_HPP

#include <queue>

namespace SystemC_VPC { namespace Detail { namespace TT {

/* struct used to store an event with a certain release-time */
struct TimeNodePair{
  TimeNodePair(sc_core::sc_time time,  TaskInterface *node)
    : time(time), node(node) {}
  sc_core::sc_time time;
  TaskInterface *node;
};

/* struct used for comparison
 * needed by the priority_queue */
struct nodeCompare{
  bool operator()(const TimeNodePair& tnp1,
                  const TimeNodePair& tnp2) const
  {
    sc_core::sc_time p1=tnp1.time;
    sc_core::sc_time p2=tnp2.time;
    if (p1 > p2)
      return true;
    else
      return false;
  }
};

typedef std::priority_queue <TimeNodePair,
                             std::vector<TimeNodePair>,
                             nodeCompare>  TimedQueue;

} } } // namespace SystemC_VPC::Detail::TT

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_TIMETRIGGERED_TT_SUPPORT_HPP */
