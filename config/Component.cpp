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

#include <systemcvpc/config/Component.hpp>
#include <systemcvpc/config/Scheduler.hpp>
#include <systemcvpc/config/Timing.hpp>
#include <systemcvpc/ScheduledTask.hpp>

#include <string>
#include <set>

namespace SystemC_VPC
{

namespace Config
{

//
Component::Component(std::string name, Scheduler::Type scheduler) :
  name_(name), scheduler_(scheduler), attribute_(new Attribute())
{
}

//
void Component::setTransferDelay(sc_core::sc_time transfer_delay)
{
  transfer_delay_ = transfer_delay;
}

//
void Component::setScheduler(Scheduler::Type scheduler)
{
  scheduler_ = scheduler;
}

//
Scheduler::Type Component::getScheduler()
{
  return scheduler_;
}

//
void Component::addTask(const ScheduledTask & actor)
{
  mappedTasks_.insert(&actor);
}

//
std::string Component::getName() const
{
  return name_;
}

//
bool Component::hasTask(const ScheduledTask * actor) const
{
  return mappedTasks_.find(actor) != mappedTasks_.end();
}

//
void Component::setTimings(Timings timings)
{
  timings_ = timings;
}

//
void Component::addTimings(std::set<Timing> timings)
{
  timings_.insert(timings.begin(), timings.end());
}

//
Component::Timings Component::getTimings()
{
  return timings_;
}

AttributePtr Component::getAttribute() const
{
  return attribute_;
}

void Component::setAttribute(AttributePtr attribute)
{
  this->attribute_ = attribute;
}

//
Component::MappedTasks Component::getMappedTasks()
{
  return mappedTasks_;
}

//
ComponentId Component::getComponentId() const
{
  //  std::cerr << " getComponentId() " << this->getSequentialId() << std::endl;
  return this->getSequentialId();
}

} // namespace Config
} // namespace SystemC_VPC

