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

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <string>

namespace SystemC_VPC
{

namespace Config
{

namespace Scheduler
{
enum Type
{
  FCFS,
  FCFS_old,
  FCFS_noTT,
  StaticPriority_P,
  StaticPriority_NP,
  DynamicPriorityUserYield,
  RoundRobin,
  RateMonotonic,
  FlexRay,
  TDMA,
  TTCC,
  AVB,
  MOST
};

Type parseScheduler(std::string name);

} // namespace Scheduler
} // namespace Config
} // namespace SystemC_VPC
#endif /* SCHEDULER_H_ */
