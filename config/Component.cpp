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

#include <boost/shared_ptr.hpp>

#include <string>
#include <set>

namespace SystemC_VPC
{

namespace Config
{

//
Component::Component(std::string name, Scheduler::Type scheduler) :
  name_(name), scheduler_(scheduler), attribute_(new Attribute()),
      componentInterface_(NULL)
{
}

//
void Component::setTransferTiming(Timing transferTiming)
{
  transferTiming_ = transferTiming;
}

//
Timing Component::getTransferTiming() const
{
  return transferTiming_;
}

//
void Component::setScheduler(Scheduler::Type scheduler)
{
  scheduler_ = scheduler;
}

//
Scheduler::Type Component::getScheduler() const
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
void Component::setTimingsProvider(TimingsProvider::Ptr provider)
{
  timingsProvider_ = provider;
}

//
TimingsProvider::Ptr Component::getTimingsProvider()
{
  if (timingsProvider_) {
    return timingsProvider_;
  } else if (defaultTimingsProvider_) {
    return defaultTimingsProvider_;
  }
  throw ConfigException("\tComponent \"" + this->name_
      + "\" has NO timing provider"
        "\n\tEither set one: Component::setTimingsProvider(TimingsProvider::Ptr )"
        "\n\tOr use default one: Component::getDefaultTimingsProvider()");
}

//
DefaultTimingsProvider::Ptr Component::getDefaultTimingsProvider()
{
  if (!defaultTimingsProvider_) {
    defaultTimingsProvider_.reset(new DefaultTimingsProvider());
  }
  return defaultTimingsProvider_;
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
  return this->getSequentialId();
}

//
ComponentInterface::Ptr Component::getComponentInterface() const
{
  //TODO: assert simulation phase
  assert(componentInterface_ != NULL);
  return componentInterface_;
}

} // namespace Config
} // namespace SystemC_VPC

