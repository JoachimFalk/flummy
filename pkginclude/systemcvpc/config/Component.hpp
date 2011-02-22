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

#include "ConfigException.hpp"
#include "Scheduler.hpp"
#include "Timing.hpp"
#include "../Attribute.hpp"
#include "../datatypes.hpp"

#include <boost/shared_ptr.hpp>

#include <systemc>
#include <set>
#include <string>

namespace SystemC_VPC
{

class ScheduledTask;

namespace Config
{

class Component : protected SequentiallyIdedObject<ComponentId>
{
public:
  typedef boost::shared_ptr<Component> Ptr;
  typedef std::set<const ScheduledTask *> MappedTasks;

  Component(std::string name, Scheduler::Type scheduler);

  void setScheduler(Scheduler::Type scheduler);

  Scheduler::Type getScheduler() const;

  void setTransferTiming(Timing transferTiming);

  Timing getTransferTiming() const;

  void addTask(const ScheduledTask & actor);

  bool hasTask(const ScheduledTask * actor) const;

  void setTimingsProvider(TimingsProvider::Ptr provider);

  TimingsProvider::Ptr getTimingsProvider();

  DefaultTimingsProvider::Ptr getDefaultTimingsProvider();

  MappedTasks getMappedTasks();

  std::string getName() const;
  AttributePtr getAttribute() const;
  void setAttribute(AttributePtr attribute);
  ComponentId getComponentId() const;
private:
  std::string name_;
  Timing transferTiming_;
  Scheduler::Type scheduler_;
  MappedTasks mappedTasks_;
  TimingsProvider::Ptr timingsProvider_;
  DefaultTimingsProvider::Ptr defaultTimingsProvider_;
  AttributePtr attribute_;
};
} // namespace Config
} // namespace SystemC_VPC
#endif /* COMPONENT_H_ */
