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

#include <systemcvpc/config/ConfigException.hpp>
#include <systemcvpc/config/Scheduler.hpp>

#include <string>

namespace SystemC_VPC
{

namespace Config
{

namespace Scheduler
{

Type parseScheduler(std::string name)
{
  static const std::string STR_TDMA = "TDMA";
  static const std::string STR_FLEXRAY = "FlexRay";
  static const std::string STR_TTCC = "TTCC";
  static const std::string STR_ROUNDROBIN = "RoundRobin";
  static const std::string STR_RR = "RR";
  static const std::string STR_PRIORITYSCHEDULER = "PriorityScheduler";
  static const std::string STR_PS = "PS";
  static const std::string STR_PRIORITYSCHEDULERNOPREEMPT =
      "PrioritySchedulerNoPreempt";
  static const std::string STR_PSNOPRE = "PSNOPRE";
  static const std::string STR_PSNOPRE_NO_TT = "PSNOPRE-noTT";
  static const std::string STR_RATEMONOTONIC = "RateMonotonic";
  static const std::string STR_RM = "RM";
  static const std::string STR_FIRSTCOMEFIRSTSERVED = "FirstComeFirstServed";
  static const std::string STR_FCFS = "FCFS";
  static const std::string STR_FCFS_OLD = "FCFS-old";
  static const std::string STR_FCFS_NO_TT = "FCFS-noTT";
  static const std::string STR_AVB = "AVB";
  static const std::string STR_MOST = "MOST";

  if (name == STR_TDMA) {
    return TDMA;
  } else if (name == STR_FLEXRAY) {
    return FlexRay;
  } else if (name == STR_TTCC) {
    return TTCC;
  } else if (name == STR_RR || name == STR_ROUNDROBIN) {
    return RoundRobin;
  } else if (name == STR_PS || name == STR_PRIORITYSCHEDULER) {
    return StaticPriority_P;
  } else if (name == STR_PSNOPRE || name == STR_PRIORITYSCHEDULERNOPREEMPT) {
    return StaticPriority_NP;
  } else if (name == STR_PSNOPRE_NO_TT) {
    return StaticPriority_NP_noTT;
  } else if (name == STR_RM || name == STR_RATEMONOTONIC) {
    return RateMonotonic;
  } else if (name == STR_FCFS || name == STR_FIRSTCOMEFIRSTSERVED) {
    return FCFS;
  } else if (name == STR_FCFS_OLD) {
    return FCFS_old;
  } else if (name == STR_FCFS_NO_TT) {
    return FCFS_noTT;
  } else if (name == STR_AVB) {
    return AVB;
  } else if (name == STR_MOST){
    return MOST;
  } else {
    throw Config::ConfigException("Unknown scheduler \"" + name
        + "\" for component: " + name);
    return FCFS;
  }

}

} // namespace Scheduler
} // namespace Config
} // namespace SystemC_VPC

