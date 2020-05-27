// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 Tim Grocki <grocki@codesign.informatik.uni-erlangen.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Martin Letras <martin.letras@fau.de>
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

#include <systemcvpc/Component.hpp>
#include <systemcvpc/ComponentObserver.hpp>
#include <systemcvpc/ComponentTracer.hpp>
#include <systemcvpc/Scheduler.hpp>
#include <systemcvpc/Timing.hpp>
#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/TimingModifier.hpp>
#include <systemcvpc/VpcApi.hpp>
#include <systemcvpc/ConfigException.hpp>

#include "detail/common.hpp"
#include "detail/Configuration.hpp"
#include "detail/AbstractComponent.hpp"
#include "detail/AbstractExecModel.hpp"
#include "detail/NonPreemptiveScheduler/DynamicPriorityComponent.hpp"
#include "detail/NonPreemptiveScheduler/FcfsComponent.hpp"
#include "detail/NonPreemptiveScheduler/NonPreemptiveComponent.hpp"
#include "detail/NonPreemptiveScheduler/PriorityComponent.hpp"
#include "detail/NonPreemptiveScheduler/RoundRobinComponent.hpp"
#include "detail/PreemptiveScheduler/PreemptiveComponent.hpp"
#include "detail/PreemptiveScheduler/AVBScheduler.hpp"
#include "detail/PreemptiveScheduler/FlexRayScheduler.hpp"
#include "detail/PreemptiveScheduler/MostScheduler.hpp"
#include "detail/PreemptiveScheduler/MostSecondaryScheduler.hpp"
#include "detail/PreemptiveScheduler/PriorityScheduler.hpp"
#include "detail/PreemptiveScheduler/RateMonotonicScheduler.hpp"
#include "detail/PreemptiveScheduler/RoundRobinScheduler.hpp"
#include "detail/PreemptiveScheduler/StreamShaperScheduler.hpp"
#include "detail/PreemptiveScheduler/TDMAScheduler.hpp"
#include "detail/PreemptiveScheduler/TimeTriggeredCCScheduler.hpp"

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

void Component::setTransferTimingModifier(TimingModifier::Ptr timingModifier) {
  transferTiming_.setTimingModifier(timingModifier);
}

//
Timing Component::getTransferTiming() const {
  return transferTiming_;
}

//
void Component::addTask(ScheduledTask &actor) {
  getTask(actor)->mapTo(this);
}

//
std::string const &Component::getName() const {
  return static_cast<Detail::AbstractComponent const *>(this)->getName();
}

PowerMode const &Component::getPowerMode() const {
  return static_cast<Detail::AbstractComponent const *>(this)->getPowerMode();
}

ComponentState   Component::getComponentState() const {
  return static_cast<Detail::AbstractComponent const *>(this)->getComponentState();
}

Power            Component::getPowerConsumption() const {
  return static_cast<Detail::AbstractComponent const *>(this)->getPowerConsumption();
}

void Component::addTracer(const char *tracerTypeOrName, Attributes const &attrs) {
  ComponentTracer::Ptr tracer;

  try {
    tracer = createComponentTracer(tracerTypeOrName, attrs);
  } catch (TracerTypeUnknown const &) {
    tracer = getComponentTracer(tracerTypeOrName);
    assert(tracer);
    for (Attribute attr : attrs) {
      tracer->addAttribute(attr);
    }
  }
  static_cast<Detail::AbstractComponent *>(this)->
      addObserver(tracer->getImpl());
}

void Component::addTracer(ComponentTracer::Ptr const &tracer) {
  static_cast<Detail::AbstractComponent *>(this)->
      addObserver(tracer->getImpl());
}

void Component::addObserver(const char *observerTypeOrName, Attributes const &attrs) {
  ComponentObserver::Ptr observer;

  try {
    observer = createComponentObserver(observerTypeOrName, attrs);
  } catch (ObserverTypeUnknown const &) {
    observer = getComponentObserver(observerTypeOrName);
    assert(observer);
    for (Attribute attr : attrs) {
      observer->addAttribute(attr);
    }
  }
  static_cast<Detail::AbstractComponent *>(this)->
      addObserver(observer->getImpl());
}

void Component::addObserver(ComponentObserver::Ptr const &observer) {
  static_cast<Detail::AbstractComponent *>(this)->
      addObserver(observer->getImpl());
}

//
bool Component::hasTask(ScheduledTask *actor) const {
  return getTask(*actor)->getComponent() == this;
}

//
void Component::setExecModel(ExecModel::Ptr model) {
  static_cast<Detail::AbstractComponent *>(this)->setExecModel(model->getImpl());
}

//
ExecModel::Ptr Component::getExecModel() {
  Detail::AbstractExecModel *execModel =
      static_cast<Detail::AbstractComponent *>(this)->getExecModel();
  if (execModel)
    return execModel->getExecModel();
  else
    return nullptr;
//throw ConfigException("\tComponent \"" + this->getName()
//    + "\" has NO timing provider!");
}

void Component::addAttribute(Attribute const &attribute) {
  if (attribute.isType("transaction_delay")) {
    sc_core::sc_time transferDelay = Detail::createSC_Time(attribute.getValue().c_str());
    this->setTransferTiming(SystemC_VPC::Timing(transferDelay));
  } else if (attribute.isType("transfer_delay")) {
    sc_core::sc_time transferDelay = Detail::createSC_Time(attribute.getValue().c_str());
    this->setTransferTiming(SystemC_VPC::Timing(transferDelay));
  } else if (attribute.isType("transaction")) {
    Attributes const           &attrs = attribute.getAttributes();
    Attributes::const_iterator  iter;
//  unsigned int transactionSize = 1;
    sc_core::sc_time transferDelay = sc_core::SC_ZERO_TIME;
    if ((iter = attrs.find("delay")) != attrs.end()) {
      transferDelay = Detail::createSC_Time(iter->getValue().c_str());
    }

//  if (attribute->hasParameter("size")) {
//    transactionSize = atoi(attribute->getParameter("size").c_str());
//  }

    this->setTransferTiming(SystemC_VPC::Timing(transferDelay));
    // FIXME: add transactionSize
  } else if (attribute.isType("tracing") || attribute.isType("tracer")) {
    addTracer(attribute.getValue().c_str(), attribute.getAttributes());
  } else if (attribute.isType("observer")) {
    addObserver(attribute.getValue().c_str(), attribute.getAttributes());
  } else if (attribute.isType("execModel")) {
    ExecModel::Ptr execModel = createExecModel(attribute.getValue().c_str());
    for (Attribute emAttr : attribute.getAttributes()) {
      if (!execModel->addAttribute(emAttr))
        throw ConfigException("Unhandled attribute " + emAttr.getType() + " for execution model " + attribute.getValue());
    }
    setExecModel(execModel);
  } else {
    throw ConfigException("Unhandled attribute " + attribute.getType());
  }
}

Component::MappedTasks Component::getMappedTasks() {
  Component::MappedTasks retval;
  for (Detail::VpcTasks::value_type const &v :
      Detail::Configuration::getInstance().getVpcTasks())
    if (v.second->getComponent() == this) {
      assert(v.second->getActor() != nullptr);
      retval.insert(const_cast<ScheduledTask *>(v.second->getActor()));
    }
  return retval;
}

ComponentId Component::getComponentId() const {
  return static_cast<Detail::AbstractComponent const *>(this)->getComponentId();
}

ComponentInterface *Component::getComponentInterface() {
  return static_cast<Detail::AbstractComponent *>(this);
}

bool hasComponent(std::string name) {
  return Detail::Configuration::getInstance().hasComponent(name).get();
}

Component::Ptr getComponent(std::string name) {
  return Detail::Configuration::getInstance().getComponent(name);
}

Component::Ptr createComponent(std::string name, Scheduler scheduler) {
  return Detail::Configuration::getInstance().createComponent(name,
      [&name, scheduler]() ->  Detail::AbstractComponent::Ptr {
      switch (scheduler) {
        case Scheduler::FCFS:
          return new Detail::FcfsComponent(name);
        case Scheduler::StaticPriorityNoPreempt:
          return new Detail::PriorityComponent(name);
        case Scheduler::RoundRobinNoPreempt:
          return new Detail::RoundRobinComponent(name);
        case Scheduler::DynamicPriorityUserYield:
          return new Detail::DynamicPriorityComponent(name);
        case Scheduler::RoundRobin:
          return new Detail::PreemptiveComponent(name, new Detail::RoundRobinScheduler());
        case Scheduler::StaticPriority:
          return new Detail::PreemptiveComponent(name, new Detail::PriorityScheduler());
        case Scheduler::RateMonotonic:
          return new Detail::PreemptiveComponent(name, new Detail::RateMonotonicScheduler());
        case Scheduler::TDMA:
          return new Detail::PreemptiveComponent(name, new Detail::TDMAScheduler());
        case Scheduler::FlexRay:
          return new Detail::PreemptiveComponent(name, new Detail::FlexRayScheduler());
        case Scheduler::AVB:
          return new Detail::PreemptiveComponent(name, new Detail::AVBScheduler());
        case Scheduler::TTCC:
          return new Detail::PreemptiveComponent(name, new Detail::TimeTriggeredCCScheduler());
        case Scheduler::MOST:
          return new Detail::PreemptiveComponent(name, new Detail::MostScheduler());
        case Scheduler::StreamShaper:
          return new Detail::PreemptiveComponent(name, new Detail::StreamShaperScheduler());
        default:
          assert(!"Oops, I don't know this scheduler!");
          throw std::runtime_error("Oops, I don't know this scheduler!");
      }
    });
}

} // namespace SystemC_VPC
