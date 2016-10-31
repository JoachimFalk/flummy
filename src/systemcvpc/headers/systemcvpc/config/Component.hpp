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

#include "common.hpp"
#include "ConfigException.hpp"
#include "Scheduler.hpp"
#include "Timing.hpp"
#include "../Attribute.hpp"
#include "../datatypes.hpp"
#include <CoSupport/SystemC/systemc_support.hpp>
#include "../TimingModifier.hpp"

#include <boost/shared_ptr.hpp>

#include <systemc>
#include <set>
#include <string>

namespace SystemC_VPC
{

class ScheduledTask;
class AbstractComponent;
class Task;

class ComponentInterface
{
public:
  typedef ComponentInterface* Ptr;

  virtual ~ComponentInterface()
  {
  }
  virtual void changePowerMode(std::string powerMode) = 0;
  virtual bool hasWaitingOrRunningTasks() = 0;
  virtual void registerComponentWakeup(const ScheduledTask * actor, Coupling::VPCEvent::Ptr event) = 0;
  virtual void registerComponentIdle(const ScheduledTask * actor, Coupling::VPCEvent::Ptr event) = 0;
  virtual void setCanExec(bool canExec) = 0;
  virtual void setDynamicPriority(std::list<ScheduledTask *>) = 0;
  virtual std::list<ScheduledTask *> getDynamicPriority() = 0;
  virtual void scheduleAfterTransition() = 0;
  virtual bool addStream(ProcessId pid){return false;};
  virtual bool closeStream(ProcessId pid){return false;};
};

namespace Config
{

class Component:
    protected SequentiallyIdedObject<ComponentId> ,
    public Traceable
{
public:
  typedef boost::shared_ptr<Component> Ptr;
  typedef std::set<ScheduledTask *> MappedTasks;

  Component(std::string name, Scheduler::Type scheduler);

  void setScheduler(Scheduler::Type scheduler);

  Scheduler::Type getScheduler() const;

  void setTransferTiming(Timing transferTiming);

  void setTransferTimingModifier(boost::shared_ptr<TimingModifier> timingModifier);

  Timing getTransferTiming() const;

  void addTask(ScheduledTask & actor);

  bool hasTask(ScheduledTask * actor) const;

  void setTimingsProvider(TimingsProvider::Ptr provider);

  TimingsProvider::Ptr getTimingsProvider();

  DefaultTimingsProvider::Ptr getDefaultTimingsProvider();

  MappedTasks getMappedTasks();

  std::string getName() const;
  std::vector<AttributePtr> getAttributes() const;
  void addAttribute(AttributePtr attribute);
  ComponentId getComponentId() const;

  ComponentInterface::Ptr getComponentInterface() const;
  bool hasDebugFile() const;
  std::string getDebugFileName() const;
  void setDebugFileName(std::string debugFileName);
private:
  friend class SystemC_VPC::AbstractComponent;

  std::string name_;
  std::string debugFileName_;
  Timing transferTiming_;
  Scheduler::Type scheduler_;
  MappedTasks mappedTasks_;
  TimingsProvider::Ptr timingsProvider_;
  DefaultTimingsProvider::Ptr defaultTimingsProvider_;
  std::vector<AttributePtr> attributes_;
  ComponentInterface::Ptr componentInterface_;
};
} // namespace Config
} // namespace SystemC_VPC
#endif /* COMPONENT_H_ */
