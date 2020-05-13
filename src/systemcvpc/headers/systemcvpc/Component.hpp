// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2011 Thomas Russ <tr.thomas.russ@googlemail.com>
 *   2012 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2012 Tim Grocki <grocki@codesign.informatik.uni-erlangen.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _INCLUDED_SYSTEMCVPC_COMPONENT_HPP
#define _INCLUDED_SYSTEMCVPC_COMPONENT_HPP

#include "Scheduler.hpp"
#include "Timing.hpp"
#include "TimingModifier.hpp"
#include "Attribute.hpp"
#include "datatypes.hpp"
#include "ScheduledTask.hpp"
#include "VPCEvent.hpp"
#include "ExecModel.hpp"
#include "PowerMode.hpp"
#include "Power.hpp"
#include "ComponentObserver.hpp"
#include "ComponentTracer.hpp"

#include <CoSupport/SmartPtr/RefCountObject.hpp>

#include <boost/noncopyable.hpp>

#include <set>
#include <string>

namespace SystemC_VPC {


enum class ComponentState { IDLE, RUNNING, STALLED };

class ComponentInterface {
public:
  typedef ComponentInterface *Ptr;

  virtual void changePowerMode(std::string powerMode) = 0;

  virtual bool hasWaitingOrRunningTasks() = 0;

  virtual void registerComponentWakeup(const ScheduledTask * actor, VPCEvent::Ptr event) = 0;
  virtual void registerComponentIdle(const ScheduledTask * actor, VPCEvent::Ptr event) = 0;

  virtual void setCanExec(bool canExec) = 0;

  virtual void                       setDynamicPriority(std::list<ScheduledTask *>) = 0;
  virtual std::list<ScheduledTask *> getDynamicPriority() = 0;

  virtual void scheduleAfterTransition() = 0;

  virtual bool addStream(ProcessId pid) = 0;
  virtual bool closeStream(ProcessId pid) = 0;

  virtual ~ComponentInterface() {}
};

class Component
  : private boost::noncopyable
  , public CoSupport::SmartPtr::RefCountObject
{
  typedef Component this_type;
public:
  typedef boost::intrusive_ptr<this_type>       Ptr;
  typedef boost::intrusive_ptr<this_type const> ConstPtr;

  typedef std::set<ScheduledTask *> MappedTasks;

  /// This is the set of built in tracers.
  struct Tracer {
    static const char *DB;   // write trace to a database
    static const char *PAJE; // trace to paje file
    static const char *VCD;  // trace to vcd file
  };

  void setTransferTiming(Timing transferTiming);

  void setTransferTimingModifier(boost::shared_ptr<TimingModifier> timingModifier);

  Timing getTransferTiming() const;

  void addTask(ScheduledTask & actor);

  bool hasTask(ScheduledTask * actor) const;

  /**
   * \brief Add a tracer. Either specify a tracer type, e.g., Tracer::PAJE,
   * or a tracer name, e.g., one given to createComponentTracer.
   */
  void addTracer(const char *tracerTypeOrName, Attributes const &attrs = Attributes());

  /**
   * \brief Add a tracer.
   */
  void addTracer(ComponentTracer::Ptr const &tracer);

  /**
   * \brief Add an observer. Either specify an observer type or name, e.g.,
   * one given to createComponentObserver.
   */
  void addObserver(const char *observerTypeOrName, Attributes const &attrs = Attributes());

  /**
   * \brief Add an observer.
   */
  void addObserver(ComponentObserver::Ptr const &observer);

  void setExecModel(ExecModel::Ptr provider);
  ExecModel::Ptr getExecModel();

  MappedTasks getMappedTasks();

  std::string const &getName() const;

  /**
   * \brief Set parameter for Component and Scheduler.
   */
  virtual void addAttribute(Attribute const &attribute);

  ComponentId getComponentId() const;

  ComponentInterface *getComponentInterface();

  virtual bool        hasDebugFile() const = 0;
  virtual void        setDebugFileName(std::string const &fileName) = 0;
  virtual std::string getDebugFileName() const = 0;

  PowerMode const &getPowerMode() const;

  ComponentState   getComponentState() const;

  Power            getPowerConsumption() const;

protected:
  Component();
private:
  Timing                      transferTiming_;
};

bool hasComponent(std::string name);

Component::Ptr createComponent(std::string name,
    Scheduler scheduler = Scheduler::FCFS);

Component::Ptr getComponent(std::string name);

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_COMPONENT_HPP */
