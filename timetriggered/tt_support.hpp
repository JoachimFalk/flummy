/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 *
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#ifndef TT_SUPPORT_HPP_
#define TT_SUPPORT_HPP_

#include <queue>

namespace SystemC_VPC
{
namespace TT
{

/* struct used to store an event with a certain release-time */
struct TimeNodePair{
  TimeNodePair(sc_time time,  ScheduledTask *node)
    : time(time), node(node) {}
  sc_time time;
  ScheduledTask *node;
};

/* struct used for comparison
 * needed by the priority_queue */
struct nodeCompare{
  bool operator()(const TimeNodePair& tnp1,
                  const TimeNodePair& tnp2) const
  {
    sc_time p1=tnp1.time;
    sc_time p2=tnp2.time;
    if (p1 > p2)
      return true;
    else
      return false;
  }
};

typedef std::priority_queue <TimeNodePair,
                             std::vector<TimeNodePair>,
                             nodeCompare>  TimedQueue;

} // namespace TT
} // namespace SystemC_VPC
#endif /* TT_SUPPORT_HPP_ */
