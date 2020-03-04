// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2020 Hardware-Software-CoDesign, University of
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

#include "config.h"

#include <systemcvpc/Timing.hpp>
#include <systemcvpc/VpcApi.hpp>
#include <systemcvpc/VpcTask.hpp>
#include <systemcvpc/PossibleAction.hpp>
#include <systemcvpc/ExecModel.hpp>

#include "common.hpp"
#include "Director.hpp"
#include "AbstractComponent.hpp"
#include "AbstractRoute.hpp"
#include "TaskImpl.hpp"
#include "HysteresisLocalGovernor.hpp"

#include "dynload/dll.hpp"
#include "PluggablePowerGovernor.hpp"

#include "ConfigCheck.hpp"

#include <cassert>

namespace SystemC_VPC { namespace Detail {

  ///
  /// Handle interfaces for SystemC_VPC::Component
  ///

  // Realize debug file interface from SystemC_VPC::Component with
  // a default unsupported implementation.
  bool        AbstractComponent::hasDebugFile() const {
    return false;
  }
  // Realize debug file interface from SystemC_VPC::Component with
  // a default unsupported implementation.
  void        AbstractComponent::setDebugFileName(std::string const &fileName) {
    throw SystemC_VPC::ConfigException(std::string("Component ") + this->name() +
        " doesn't support specification of a debug file!");
  }
  // Realize debug file interface from SystemC_VPC::Component with
  // a default unsupported implementation.
  std::string AbstractComponent::getDebugFileName() const {
    throw SystemC_VPC::ConfigException(std::string("Component ") + this->name() +
        " doesn't support specification of a debug file!");
  }

  ///
  /// Handle interfaces for SystemC_VPC::ComponentInterface
  ///

  void AbstractComponent::changePowerMode(std::string powerMode) {
    execModel->setPowerMode(execModelComponentState, powerMode);
    this->powerMode = powerMode;
    this->componentOperation(ComponentOperation::PWRCHANGE, *this);
  }

  void AbstractComponent::registerComponentWakeup(const ScheduledTask * actor, VPCEvent::Ptr event){
    componentWakeup = event;
  }

  void AbstractComponent::registerComponentIdle(const ScheduledTask * actor, VPCEvent::Ptr event){
    componentIdle = event;
  }

  void AbstractComponent::setCanExec(bool canExec){
    this->setCanExecuteTasks(canExec);
  }

  void AbstractComponent::setDynamicPriority(std::list<ScheduledTask *> priorityList) {
    throw SystemC_VPC::ConfigException(std::string("Component ") + this->name() +
        " doesn't support dynamic priorities!");
  }

  std::list<ScheduledTask *> AbstractComponent::getDynamicPriority() {
    throw SystemC_VPC::ConfigException(std::string("Component ") + this->name() +
        " doesn't support dynamic priorities!");
  }

  void AbstractComponent::scheduleAfterTransition() {
    throw SystemC_VPC::ConfigException(std::string("Component ") + this->name() +
        " doesn't support scheduleAfterTransition()!");
  }

  bool AbstractComponent::addStream(ProcessId pid) {
    return false;
  }

  bool AbstractComponent::closeStream(ProcessId pid) {
    return false;
  }

  ///
  /// Other stuff
  ///

  AbstractComponent::Factories AbstractComponent::factories;


  AbstractComponent::~AbstractComponent() {
    componentOperation(ComponentOperation::PWRCHANGE, *this);
  }

  void AbstractComponent::loadLocalGovernorPlugin(std::string plugin){
    //std::cerr << "Component::loadLocalGovernorPlugin" << std::endl;

    if(plugin == "") return;

    if(AbstractComponent::factories.find(plugin)
        == AbstractComponent::factories.end()){
      AbstractComponent::factories[plugin] =
        new DLLFactory<PlugInFactory<PluggableLocalPowerGovernor> >
          (plugin.c_str());
    }

    localGovernorFactory = AbstractComponent::factories[plugin]->factory;
  }

  void AbstractComponent::requestCanExecute(){
    //assert(this->canExecuteTasks == false);
    componentIdle->reset();
    if(!requestExecuteTasks && componentWakeup != 0){
      //std::cout<< "Comp: " << this->getName()<<" requestCanExecute() - componentWakeup->notify() @ " << sc_core::sc_time_stamp() <<  std::endl;
      //First request
      requestExecuteTasks=true;
      componentWakeup->notify();
    }
  }

  bool AbstractComponent::requestShutdown(){
      //FIXME: why did I use sc_pending_activity_at_current_time() here? what special-case?
    if(!hasWaitingOrRunningTasks() && (shutdownRequestAtTime == sc_core::sc_time_stamp()) /*&& !sc_pending_activity_at_current_time()*/){
      if(componentIdle != 0){
        //std::cout<< "Comp: " << this->getName()<<" requestShutdown() - componentIdle->notify() @ " << sc_core::sc_time_stamp() << " hasWaitingOrRunningTasks=" << hasWaitingOrRunningTasks()<< " " << sc_pending_activity_at_current_time() /*<< " " << m_simcontext->next_time()*/ <<  std::endl;
        //TODO: maybe notify it in the future?
        componentIdle->notify();
        if(sc_core::sc_pending_activity_at_current_time()){
            return false;
        }
      }
      return true;
    }else{
      shutdownRequestAtTime = sc_core::sc_time_stamp();
      return false;
    }
  }

  void AbstractComponent::initialize(const Director *d) {
  }

  bool AbstractComponent::getCanExecuteTasks() const {
      return canExecuteTasks;
  }

  void AbstractComponent::setCanExecuteTasks(bool canExecuteTasks) {
    bool oldCanExecuteTasks = this->canExecuteTasks;
    this->canExecuteTasks = canExecuteTasks;
    if(!oldCanExecuteTasks && this->canExecuteTasks){
      requestExecuteTasks = false;
      this->reactivateExecution();
    }
  }

  void AbstractComponent::reactivateExecution() {

  }

  AbstractComponent::AbstractComponent(std::string const &name)
    : sc_core::sc_module(sc_core::sc_module_name(name.c_str()))
    , requestExecuteTasks(false)
    , localGovernorFactory(nullptr)
    , midPowerGov(nullptr)
    , powerAttribute(new Attribute("",""))
    , componentName(name)
    , canExecuteTasks(true)
    , execModelComponentState(nullptr)
    , assignedTaskInstance(nullptr)
    , latencyQueue("latencyQueue"
        , [this] (TaskInstanceImpl *ti) { this->finishLatencyTaskInstance(ti); })
    , powerMode(PowerMode::DEFAULT)
    , compState(ComponentState::IDLE)
    , powerConsumption(0)
    {}


  void AbstractComponent::setExecModel(AbstractExecModel *model) {
    assert(execModelComponentState == nullptr);
    execModelComponentState = model->attachToComponent(this);
    execModel.reset(model);
  }

  void AbstractComponent::registerTask(TaskInterface *task) {
    // This should be the first time the actor appeared here.
    VpcTask::Ptr         vpcTask = SystemC_VPC::getTask(static_cast<ScheduledTask &>(*task));
    TaskImpl *taskImpl = this->createTask(task);
    taskImpl->setPriority(vpcTask->getPriority());
    taskImpl->setTaskIsPSM(vpcTask->isPSM());
    task->setScheduler(this);
    task->setSchedulerInfo(taskImpl);
  }

  void AbstractComponent::registerFiringRule(TaskInterface *actor, PossibleAction *fr) {
    assert(actor->getScheduler() == this);
    fr->setSchedulerInfo(getExecModel()->registerAction(execModelComponentState, actor, fr));
  }

  void AbstractComponent::checkFiringRule(TaskInterface *task, PossibleAction *fr) {
    AbstractExecModel::ActionInfo *ai =
        static_cast<AbstractExecModel::ActionInfo *>(fr->getSchedulerInfo());

    TaskImpl *taskImpl = getTaskOfTaskInterface(task);
    std::function<void (TaskInstanceImpl *)> none([](TaskInstanceImpl *) {});

    TaskInstanceImpl *taskInstanceImpl = createTaskInstance(taskImpl
        , TaskInstanceImpl::Type::GUARD, fr,  none, none);
    getExecModel()->initTaskInstance(execModelComponentState, ai, taskInstanceImpl, true);
    this->check(taskInstanceImpl);
  }

  class AbstractComponent::InputsAvailableListener {
  public:
    InputsAvailableListener(TaskInstanceImpl *taskInstanceImpl)
      : taskInstanceImpl(taskInstanceImpl)
      // We start with one more, i.e., +1, to ensure that
      // arrived does not call compute before wait is called!
      , missing(taskInstanceImpl->getFiringRule()->getPortInInfos().size()+1) {}

    void wait() {
      if (!--missing)
        compute();
    }

    void arrived() {
      if (!--missing)
        compute();
    }
  private:
    TaskInstanceImpl *taskInstanceImpl;
    size_t            missing;

    void compute() {
      TaskImpl          *taskImpl = taskInstanceImpl->getTask();
      assert(taskImpl != NULL);
      TaskInterface     *task     = taskImpl->getScheduledTask();
      assert(task != NULL);
      AbstractComponent *comp     = static_cast<AbstractComponent *>(task->getScheduler());
      PossibleAction    *fr       = taskInstanceImpl->getFiringRule();
      assert(fr != NULL);
      AbstractExecModel::ActionInfo *ai =
          static_cast<AbstractExecModel::ActionInfo *>(fr->getSchedulerInfo());

      comp->getExecModel()->initTaskInstance(comp->execModelComponentState, ai, taskInstanceImpl);
      comp->compute(taskInstanceImpl);
      delete this;
    }
  };

  void AbstractComponent::executeFiringRule(TaskInterface *task, PossibleAction *fr) {
    std::function<void (TaskInstanceImpl *)> none([](TaskInstanceImpl *) {});

    TaskImpl         *taskImpl = getTaskOfTaskInterface(task);
    TaskInstanceImpl *taskInstanceImpl = createTaskInstance(taskImpl
        , TaskInstanceImpl::Type::ACTION, fr, none, none);

    InputsAvailableListener *ial = new InputsAvailableListener(taskInstanceImpl);

    for (PossibleAction::PortInInfo const &portInfo : fr->getPortInInfos()) {
      portInfo.port.commStart(portInfo.consumed);
      AbstractRoute *route = getAbstractRouteOfPort(portInfo.port);
      route->start<InputsAvailableListener>(portInfo.required, ial,
          [](InputsAvailableListener *ial, size_t n, smoc::SimulatorAPI::ChannelSourceInterface *csi) {
            ial->arrived();
          });
    }
    for (PossibleAction::PortOutInfo const &portInfo : fr->getPortOutInfos()) {
      portInfo.port.commStart(portInfo.produced);
    }
    ial->wait();
  }

  TaskInstanceImpl *AbstractComponent::executeHop(TaskImpl *taskImpl
    , Timing const &transferTiming
    , size_t quantum
    , std::function<void (TaskInstanceImpl *)> const &cb)
  {
    assert(taskImpl != NULL);
    std::function<void (TaskInstanceImpl *)> none([](TaskInstanceImpl *) {});
    TaskInstanceImpl *taskInstance = createTaskInstance(taskImpl
        , TaskInstanceImpl::Type::MESSAGE, nullptr, none, cb);
    // FIXME: Timing independent of power mode at the moment!
    taskInstance->setDelay(quantum * transferTiming.getDii());
    taskInstance->setRemainingDelay(taskInstance->getDelay());
    taskInstance->setLatency(quantum * transferTiming.getLatency());
    this->compute(taskInstance);
    return taskInstance;
  }

  /// Called once per actor firing to indicate that the task instance is ready to execute on resource.
  void AbstractComponent::releaseTaskInstance(TaskInstanceImpl *ti) {
    this->taskInstanceOperation(TaskInstanceOperation::RELEASE
        , *this, *ti);
  }

  /// Called possibly multiple times to assign the task instance to the resource.
  void AbstractComponent::assignTaskInstance(TaskInstanceImpl *ti) {
    assert(!assignedTaskInstance);
    compState = ti->isPSM()
        // Assuming PSM actors are assigned to the same component they model,
        // the executing state of the component should be IDLE.
        ? ComponentState::IDLE
        : ComponentState::RUNNING;
    assignedTaskInstance = ti;
    assignedTaskInstanceTime = sc_core::sc_time_stamp();
    this->taskInstanceOperation(TaskInstanceOperation::ASSIGN
        , *this, *ti);
  }

  /// Called possibly multiple times to adjust remaining delay of assigned task instance.
  void AbstractComponent::ranTaskInstance(TaskInstanceImpl *ti) {
    assert(assignedTaskInstance == ti);
    sc_core::sc_time now = sc_core::sc_time_stamp();
    ti->setRemainingDelay(ti->getRemainingDelay()
        - now + assignedTaskInstanceTime);
    assignedTaskInstanceTime = now;
  }

  /// Called possibly multiple times to resign the task instance from the resource.
  void AbstractComponent::resignTaskInstance(TaskInstanceImpl *ti) {
    assert(assignedTaskInstance == ti);
    this->taskInstanceOperation(TaskInstanceOperation::RESIGN
        , *this, *ti);
    assignedTaskInstance = nullptr;
  }

  /// Called possibly multiple times to indicate that the task is blocked waiting for something.
  void AbstractComponent::blockTaskInstance(TaskInstanceImpl *ti) {
    assert(assignedTaskInstance == ti);
    compState = ComponentState::STALLED;
    this->taskInstanceOperation(TaskInstanceOperation::BLOCK
        , *this, *ti);
    assignedTaskInstance = nullptr;
  }

  /// Called once per actor firing to indicate that the DII of the task instance is over.
  void AbstractComponent::finishDiiTaskInstance(TaskInstanceImpl *ti) {
    assert(assignedTaskInstance == ti);
    compState = ComponentState::IDLE;
//  this->Tracing::TraceableComponent::finishDiiTaskInstance(taskInstance);
    ti->diiExpired();

    if (ti->getType() == TaskInstanceImpl::Type::ACTION) {
      PossibleAction *fr = ti->getFiringRule();
      assert(fr != nullptr);
      for (PossibleAction::PortInInfo const &portInfo : fr->getPortInInfos()) {
        portInfo.port.getSource()->commFinish(portInfo.consumed);
      }
    }
    this->taskInstanceOperation(TaskInstanceOperation::FINISHDII
        , *this, *ti);
    assignedTaskInstance = nullptr;
    latencyQueue.add(ti, ti->getLatency() - ti->getDelay());
  }

  /// Called once per actor firing to indicate that the latency of the task instance is over.
  void AbstractComponent::finishLatencyTaskInstance(TaskInstanceImpl *ti) {
    // Remember last acknowledged task time
    Director::getInstance().end = sc_core::sc_time_stamp();
//  this->Tracing::TraceableComponent::finishLatencyTaskInstance(taskInstance);
    ti->latExpired();

    if (ti->getType() == TaskInstanceImpl::Type::ACTION) {
      PossibleAction *fr = ti->getFiringRule();
      assert(fr != nullptr);
      for (PossibleAction::PortOutInfo const &portInfo : fr->getPortOutInfos()) {
        AbstractRoute *route = getAbstractRouteOfPort(portInfo.port);
        route->start<void>(portInfo.produced, nullptr, [](void *, size_t n, smoc::SimulatorAPI::ChannelSinkInterface *out) {
            out->commFinish(n);
          });
      }
    }
    // This may deallocate tII
    this->taskInstanceOperation(TaskInstanceOperation::FINISHLAT
        , *this, *ti);
  }


} } // namespace SystemC_VPC::Detail
