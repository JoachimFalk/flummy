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
#include <string>
#include <systemcvpc/config/ConfigException.hpp>
#include <systemcvpc/config/Scheduler.hpp>
#include <systemcvpc/Attribute.hpp>

namespace SystemC_VPC
{

class ScheduledTask;

namespace Config
{

class Timing;

class Component
{
public:
  typedef boost::shared_ptr<Component> Ptr;
  typedef std::set<Timing> Timings;
  typedef std::set<const ScheduledTask *> MappedTasks;

  Component(std::string name, Scheduler::Type scheduler);

  void setScheduler(Scheduler::Type scheduler);

  Scheduler::Type getScheduler();

  void setTransferDelay(sc_core::sc_time transfer_delay);

  void addTask(const ScheduledTask & actor);

  bool hasTask(const ScheduledTask * actor) const;

  void setTimings(std::set<Timing> timings);

  void addTimings(std::set<Timing> timings);

  Timings getTimings();

  MappedTasks getMappedTasks();

  std::string getName() const;
  AttributePtr getAttribute() const;
  void setAttribute(AttributePtr attribute);
private:
  std::string name_;
  sc_core::sc_time transfer_delay_;
  Scheduler::Type scheduler_;
  MappedTasks mappedTasks_;
  Timings timings_;
  AttributePtr attribute_;
};
} // namespace Config
} // namespace SystemC_VPC
#endif /* COMPONENT_H_ */
