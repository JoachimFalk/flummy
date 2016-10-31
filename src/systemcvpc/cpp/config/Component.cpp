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

#include <systemcvpc/config/common.hpp>
#include <systemcvpc/config/Component.hpp>
#include <systemcvpc/config/Scheduler.hpp>
#include <systemcvpc/config/Timing.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/TimingModifier.hpp>

#include <boost/shared_ptr.hpp>

#include <string>
#include <set>

namespace SystemC_VPC
{

namespace Config
{

//
Component::Component(std::string name, Scheduler::Type scheduler) :
  name_(name), debugFileName_(""), scheduler_(scheduler), componentInterface_(NULL)
{
}

//
void Component::setTransferTiming(Timing transferTiming)
{
  transferTiming_ = transferTiming;
}

void Component::setTransferTimingModifier(boost::shared_ptr<TimingModifier> timingModifier)
{
	transferTiming_.setTimingModifier(timingModifier);
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
void Component::addTask(ScheduledTask & actor)
{
  mappedTasks_.insert(&actor);
}

//
std::string Component::getName() const
{
  return name_;
}

//
bool Component::hasTask(ScheduledTask * actor) const
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

std::vector<AttributePtr> Component::getAttributes() const
{
  return attributes_;
}

void Component::addAttribute(AttributePtr attribute)
{
  if (attribute->isType("transaction_delay")) {
    sc_time transferDelay = Director::createSC_Time(attribute->getValue());
    this->setTransferTiming(Config::Timing(transferDelay));
  } else if (attribute->isType("transfer_delay")) {
    sc_time transferDelay = Director::createSC_Time(attribute->getValue());
    this->setTransferTiming(Config::Timing(transferDelay));
  } else if (attribute->isType("transaction")) {
    unsigned int transactionSize = 1;
    sc_time transferDelay = SC_ZERO_TIME;
    if (attribute->hasParameter("delay")) {
      transferDelay = Director::createSC_Time(attribute->getParameter("delay"));
    }

    if (attribute->hasParameter("size")) {
      transactionSize = atoi(attribute->getParameter("size").c_str());
    }

    this->setTransferTiming(Config::Timing(transferDelay));
    // FIXME: add transactionSize

  } else if (attribute->isType("tracing")) {
    this->setTracing(Traceable::parseTracing(attribute->getValue()));
	} else {
    this->attributes_.push_back(attribute);
  }
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

//
bool Component::hasDebugFile() const
{
  return (this->debugFileName_ != "");
}

//
std::string Component::getDebugFileName() const
{
  return this->debugFileName_;
}

//
void Component::setDebugFileName(std::string debugFileName)
{
  this->debugFileName_ = debugFileName;
}

} // namespace Config
} // namespace SystemC_VPC

