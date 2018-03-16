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
Component::Component(std::string name, Scheduler scheduler) :
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
void Component::setScheduler(Scheduler scheduler)
{
  scheduler_ = scheduler;
}

//
Scheduler Component::getScheduler() const
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
    sc_core::sc_time transferDelay = Director::createSC_Time(attribute->getValue());
    this->setTransferTiming(Config::Timing(transferDelay));
  } else if (attribute->isType("transfer_delay")) {
    sc_core::sc_time transferDelay = Director::createSC_Time(attribute->getValue());
    this->setTransferTiming(Config::Timing(transferDelay));
  } else if (attribute->isType("transaction")) {
//  unsigned int transactionSize = 1;
    sc_core::sc_time transferDelay = sc_core::SC_ZERO_TIME;
    if (attribute->hasParameter("delay")) {
      transferDelay = Director::createSC_Time(attribute->getParameter("delay"));
    }

//  if (attribute->hasParameter("size")) {
//    transactionSize = atoi(attribute->getParameter("size").c_str());
//  }

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

