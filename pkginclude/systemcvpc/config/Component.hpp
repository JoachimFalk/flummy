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

#ifndef COMPONENT_H_
#define COMPONENT_H_

#include <boost/shared_ptr.hpp>
#include <systemc>
#include <set>

namespace SystemC_VPC
{

class ScheduledTask;

namespace Config
{

class Timing;

namespace Scheduler{
enum Scheduler{
  FCFS,
  PS
};
}

class Component
{
public:
  typedef boost::shared_ptr<Component> Ptr;

  Component(std::string name, Scheduler::Scheduler scheduler);

  void setTransferDelay(sc_core::sc_time transfer_delay);

  void setScheduler(Scheduler::Scheduler scheduler);

  void addTask(const ScheduledTask & actor);

  void setTimings(std::set<Timing> timings);

};
} // namespace Config
} // namespace SystemC_VPC
#endif /* COMPONENT_H_ */
