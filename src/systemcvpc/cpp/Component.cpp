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

#include <systemcvpc/Component.hpp>
#include <systemcvpc/Scheduler.hpp>
#include <systemcvpc/Timing.hpp>
#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/TimingModifier.hpp>
#include <systemcvpc/Mappings.hpp>
#include <systemcvpc/VpcApi.hpp>

#include "detail/common.hpp"
#include "detail/AbstractComponent.hpp"

#include <boost/shared_ptr.hpp>

#include <string>
#include <set>

namespace SystemC_VPC {

//
Component::Component() {
}

//
void Component::setTransferTiming(Timing transferTiming) {
  transferTiming_ = transferTiming;
}

void Component::setTransferTimingModifier(boost::shared_ptr<TimingModifier> timingModifier) {
  transferTiming_.setTimingModifier(timingModifier);
}

//
Timing Component::getTransferTiming() const {
  return transferTiming_;
}

//
void Component::addTask(ScheduledTask &actor) {
  Mappings::getConfiguredMappings()[getTask(actor)] = this;
}

//
std::string Component::getName() const {
  return static_cast<Detail::AbstractComponent const *>(this)->getName();
}

void Component::addTracer(const char *tracer) {
  static_cast<Detail::AbstractComponent *>(this)->addTracer(tracer, this);
}

//
bool Component::hasTask(ScheduledTask *actor) const {
  return Mappings::isMapped(getTask(*actor), Ptr(const_cast<this_type *>(this)));
}

//
void Component::setTimingsProvider(TimingsProvider::Ptr provider) {
  timingsProvider_ = provider;
}

//
TimingsProvider::Ptr Component::getTimingsProvider() {
  if (timingsProvider_) {
    return timingsProvider_;
  } else if (defaultTimingsProvider_) {
    return defaultTimingsProvider_;
  }
  throw ConfigException("\tComponent \"" + this->getName()
      + "\" has NO timing provider"
        "\n\tEither set one: Component::setTimingsProvider(TimingsProvider::Ptr )"
        "\n\tOr use default one: Component::getDefaultTimingsProvider()");
}

//
DefaultTimingsProvider::Ptr Component::getDefaultTimingsProvider() {
  if (!defaultTimingsProvider_) {
    defaultTimingsProvider_.reset(new DefaultTimingsProvider());
  }
  return defaultTimingsProvider_;
}

void Component::addAttribute(AttributePtr attribute) {
  if (attribute->isType("transaction_delay")) {
    sc_core::sc_time transferDelay = Detail::createSC_Time(attribute->getValue().c_str());
    this->setTransferTiming(SystemC_VPC::Timing(transferDelay));
  } else if (attribute->isType("transfer_delay")) {
    sc_core::sc_time transferDelay = Detail::createSC_Time(attribute->getValue().c_str());
    this->setTransferTiming(SystemC_VPC::Timing(transferDelay));
  } else if (attribute->isType("transaction")) {
//  unsigned int transactionSize = 1;
    sc_core::sc_time transferDelay = sc_core::SC_ZERO_TIME;
    if (attribute->hasParameter("delay")) {
      transferDelay = Detail::createSC_Time(attribute->getParameter("delay").c_str());
    }

//  if (attribute->hasParameter("size")) {
//    transactionSize = atoi(attribute->getParameter("size").c_str());
//  }

    this->setTransferTiming(SystemC_VPC::Timing(transferDelay));
    // FIXME: add transactionSize
  } else if (attribute->isType("tracing")) {
    static_cast<Detail::AbstractComponent *>(this)->addTracer(attribute->getValue().c_str(), this);
  } else if (!static_cast<Detail::AbstractComponent *>(this)->setAttribute(attribute)) {
    throw std::runtime_error("Unhandled attribute");
  }
}

Component::MappedTasks Component::getMappedTasks() {
  Component::MappedTasks retval;
  for (std::pair<VpcTask::Ptr, Component::Ptr> const &v :
          Mappings::getConfiguredMappings())
    if (v.second == this) {
      assert(v.first->getActor() != nullptr);
      retval.insert(const_cast<ScheduledTask *>(v.first->getActor()));
    }
  return retval;
}

ComponentId Component::getComponentId() const {
  return static_cast<Detail::AbstractComponent const *>(this)->getComponentId();
}

ComponentInterface *Component::getComponentInterface() {
  return static_cast<Detail::AbstractComponent *>(this);
}

} // namespace SystemC_VPC
