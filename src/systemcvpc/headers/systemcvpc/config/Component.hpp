// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SYSTEMCVPC_CONFIG_COMPONENT_HPP
#define _INCLUDED_SYSTEMCVPC_CONFIG_COMPONENT_HPP

#include "common.hpp"
#include "ConfigException.hpp"
#include "Scheduler.hpp"
#include "Timing.hpp"
#include "../Attribute.hpp"
#include "../datatypes.hpp"
#include "../TimingModifier.hpp"
#include "../ScheduledTask.hpp"
#include "../coupling/VPCEvent.hpp"

#include <CoSupport/SystemC/systemc_support.hpp>

#include <boost/shared_ptr.hpp>

#include <systemc>
#include <set>
#include <string>

namespace SystemC_VPC { namespace Detail {

class AbstractComponent;
class TaskInstance;

} } // namespace SystemC_VPC::Detail

namespace SystemC_VPC { namespace Config {

class ComponentInterface {
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

class Component:
    protected SequentiallyIdedObject<ComponentId> ,
    public Traceable
{
public:
  typedef boost::shared_ptr<Component> Ptr;
  typedef std::set<ScheduledTask *> MappedTasks;

  Component(std::string name, Scheduler scheduler);

  void setScheduler(Scheduler scheduler);

  Scheduler getScheduler() const;

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
  friend class Detail::AbstractComponent;

  std::string name_;
  std::string debugFileName_;
  Timing      transferTiming_;
  Scheduler   scheduler_;
  MappedTasks mappedTasks_;
  TimingsProvider::Ptr timingsProvider_;
  DefaultTimingsProvider::Ptr defaultTimingsProvider_;
  std::vector<AttributePtr> attributes_;
  ComponentInterface::Ptr componentInterface_;
};

} } // namespace SystemC_VPC::Config

#endif /* _INCLUDED_SYSTEMCVPC_CONFIG_COMPONENT_HPP */
