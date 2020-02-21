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

#include "config.h"

#include <systemcvpc/Timing.hpp>
#include <systemcvpc/VpcApi.hpp>
#include <systemcvpc/VpcTask.hpp>
#include <systemcvpc/PossibleAction.hpp>

#include "common.hpp"
#include "Director.hpp"
#include "AbstractComponent.hpp"
#include "AbstractRoute.hpp"
#include "ProcessControlBlock.hpp"
#include "ComponentObserver.hpp"
#include "ComponentInfo.hpp"
#include "HysteresisLocalGovernor.hpp"

#include "dynload/dll.hpp"
#include "PluggablePowerGovernor.hpp"

#include "tracing/ComponentTracerIf.hpp"

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
  }

  void AbstractComponent::fireStateChanged(ComponentState state)
  {
    compState = state;
    // FIXME: update powerConsumption
    //this->setPowerConsumption(powerTables[getPowerMode()][getComponentState()]);
    // Notify observers (e.g. powersum)
    this->fireNotification(this);
  }


  bool AbstractComponent::processPower(AttributePtr attPtr)
  {
    // hierarchical format
    if(!attPtr->isType("powermode")) {
      return false;
    }

    for(size_t i=0; i<attPtr->getAttributeSize();++i){
      AttributePtr powerAtt = attPtr->getNextAttribute(i).second;
      if(powerAtt->isType("governor")){
        this->loadLocalGovernorPlugin(powerAtt->getValue());
        powerAttribute = powerAtt;
        continue;
      }

//    std::string powerMode = attPtr->getNextAttribute(i).first;
//    const PowerMode *power = this->translatePowerMode(powerMode);
//
//    if(powerTables.find(power) == powerTables.end()){
//      powerTables[power] = PowerTable();
//    }
//
//    PowerTable &powerTable=powerTables[power];
//
//    if(powerAtt->hasParameter("IDLE")){
//      std::string v = powerAtt->getParameter("IDLE");
//      const double value = atof(v.c_str());
//      powerTable[ComponentState::IDLE] = value;
//    }
//    if(powerAtt->hasParameter("RUNNING")){
//      std::string v = powerAtt->getParameter("RUNNING");
//      const double value = atof(v.c_str());
//      powerTable[ComponentState::RUNNING] = value;
//    }
//    if(powerAtt->hasParameter("STALLED")){
//      std::string v = powerAtt->getParameter("STALLED");
//      const double value = atof(v.c_str());
//      powerTable[ComponentState::STALLED] = value;
//    }
//    if(powerAtt->hasParameter("transaction_delay")) {
//      this->transactionDelays[power] =
//        createSC_Time(powerAtt->getParameter("transaction_delay").c_str());
//    }
//    if(powerAtt->hasParameter("transfer_delay")) {
//      this->transactionDelays[power] =
//        createSC_Time(powerAtt->getParameter("transfer_delay").c_str());
//    }

    }

    return true;
  }

  /**
   *
   */
  bool AbstractComponent::setAttribute(AttributePtr attribute){
    if (processPower(attribute)) {
      return true;
    }
    return false;
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
    , powerMode(PowerMode::DEFAULT)
    , compState(ComponentState::IDLE)
    , powerConsumption(0)
  {
//  if(powerTables.find(getPowerMode()) == powerTables.end()){
//    powerTables[getPowerMode()] = PowerTable();
//  }
//
//  PowerTable &powerTable=powerTables[getPowerMode()];
//  powerTable[ComponentState::IDLE]    = 0.0;
//  powerTable[ComponentState::RUNNING] = 1.0;
  }


  void AbstractComponent::end_of_elaboration() {
    sc_core::sc_module::end_of_elaboration();
    startTracing();
  }

  /**
   * \brief Create the process control block.
   */
  ProcessControlBlock *AbstractComponent::createPCB(std::string const &taskName) {
    ProcessControlBlock *pcb = new ProcessControlBlock(this, taskName);
    sassert(pcbPool.insert(std::make_pair(
        pcb->getPid(),
        ProcessControlBlockPtr(pcb))).second);
    this->TraceableComponent::registerTask(pcb, taskName);
    return pcb;
  }

  ProcessControlBlock *AbstractComponent::getPCB(ProcessId const pid) const {
    PCBPool::const_iterator iter = pcbPool.find(pid);
    assert(iter != pcbPool.end());
    return iter->second.get();
  }

  void AbstractComponent::setExecModel(AbstractExecModel *model) {
    assert(execModelComponentState == nullptr);
    execModelComponentState = model->attachToComponent(this);
    execModel.reset(model);
  }

  void AbstractComponent::registerTask(TaskInterface *task) {
    // This should be the first time the actor appeared here.
    VpcTask::Ptr         vpcTask = getTask(static_cast<ScheduledTask &>(*task));
    ProcessControlBlock *pcb = this->createPCB(task->name());
    pcb->setScheduledTask(task);
//  pcb->configure(task->name(), true);
    pcb->setPriority(vpcTask->getPriority());
    pcb->setTaskIsPSM(vpcTask->isPSM());
    task->setScheduler(this);
    task->setSchedulerInfo(pcb);
  }

  void AbstractComponent::registerFiringRule(TaskInterface *actor, PossibleAction *fr) {
    assert(actor->getScheduler() == this);
    fr->setSchedulerInfo(getExecModel()->registerAction(execModelComponentState, actor, fr));
  }

  void AbstractComponent::checkFiringRule(TaskInterface *task, PossibleAction *fr) {
    AbstractExecModel::ActionInfo *ai =
        static_cast<AbstractExecModel::ActionInfo *>(fr->getSchedulerInfo());

    ProcessControlBlock *pcb = getPCBOfTaskInterface(task);
    std::function<void (TaskInstance *)> none([](TaskInstance *) {});
    TaskInstance taskInstance(none, none);
    taskInstance.setPCB(pcb);
    taskInstance.setFiringRule(fr);
    taskInstance.setName(task->name()+std::string("_check"));
    getExecModel()->initTaskInstance(execModelComponentState, ai, &taskInstance, true);
    this->check(&taskInstance);
  }

  class AbstractComponent::InputsAvailableListener {
  public:
    InputsAvailableListener(
        TaskInterface                           *task,
        PossibleAction *fr)
      : task(task), fr(fr)
      // We start with one more, i.e., +1, to ensure that
      // arrived does not call compute before wait is called!
      , missing(fr->getPortInInfos().size()+1) {}

    void wait() {
      if (!--missing)
        compute();
    }

    void arrived() {
      if (!--missing)
        compute();
    }
  private:
    TaskInterface  *task;
    PossibleAction *fr;
    size_t          missing;

    void compute() {
      AbstractExecModel::ActionInfo *ai =
          static_cast<AbstractExecModel::ActionInfo *>(fr->getSchedulerInfo());
      AbstractComponent   *comp  = static_cast<AbstractComponent *>(task->getScheduler());
      ProcessControlBlock *pcb   = getPCBOfTaskInterface(task);

      assert(pcb != NULL);

      std::function<void (TaskInstance *)> none([](TaskInstance *) {});
      TaskInstance *taskInstance = new TaskInstance(none, none);
      taskInstance->setPCB(pcb);
      taskInstance->setFiringRule(fr);
      taskInstance->setName(task->name());

      comp->getExecModel()->initTaskInstance(comp->execModelComponentState, ai, taskInstance);
      comp->compute(taskInstance);
      delete this;
    }
  };

  void AbstractComponent::executeFiringRule(TaskInterface *task, PossibleAction *fr) {

    typedef PossibleAction::PortInInfo             PortInInfo;
    typedef PossibleAction::PortOutInfo            PortOutInfo;

    InputsAvailableListener *ial = new InputsAvailableListener(task, fr);

    for (PortInInfo const &portInfo : fr->getPortInInfos()) {
      portInfo.port.commStart(portInfo.consumed);
      AbstractRoute *route = getAbstractRouteOfPort(portInfo.port);
      route->start<InputsAvailableListener>(portInfo.required, ial,
          [](InputsAvailableListener *ial, size_t n, smoc::SimulatorAPI::ChannelSourceInterface *csi) {
            ial->arrived();
          });
    }
    for (PortOutInfo const &portInfo : fr->getPortOutInfos()) {
      portInfo.port.commStart(portInfo.produced);
    }
    ial->wait();
  }

  TaskInstance *AbstractComponent::executeHop(ProcessControlBlock *pcb
    , Timing const &transferTiming
    , size_t quantum
    , std::function<void (TaskInstance *)> const &cb)
  {
    assert(pcb != NULL);
    std::function<void (TaskInstance *)> none([](TaskInstance *) {});
    TaskInstance *taskInstance = new TaskInstance(none, cb);
    taskInstance->setPCB(pcb);
    taskInstance->setName(pcb->getName());
    taskInstance->setTimingScale(quantum);
    // FIXME: Timing independent of power mode at the moment!
    taskInstance->setDelay(quantum * transferTiming.getDii());
    taskInstance->setRemainingDelay(taskInstance->getDelay());
    taskInstance->setLatency(quantum * transferTiming.getLatency());
    this->compute(taskInstance);
    return taskInstance;
  }

  /// Called once per actor firing to indicate that the DII of the task instance is over.
  void AbstractComponent::finishDiiTaskInstance(TaskInstance *taskInstance, bool isGuard) {
    this->Tracing::TraceableComponent::finishDiiTaskInstance(taskInstance);

    taskInstance->diiExpired();

    if (isGuard)
      return;

    typedef PossibleAction::PortInInfo         PortInInfo;

    if (PossibleAction *fr = taskInstance->getFiringRule()) {
      for (PortInInfo const &portInfo : fr->getPortInInfos()) {
        portInfo.port.getSource()->commFinish(portInfo.consumed);
      }
    }
  }

  /// Called once per actor firing to indicate that the latency of the task instance is over.
  void AbstractComponent::finishLatencyTaskInstance(TaskInstance *taskInstance, bool isGuard) {
    // Remember last acknowledged task time
    Director::getInstance().end = sc_core::sc_time_stamp();
    this->Tracing::TraceableComponent::finishLatencyTaskInstance(taskInstance);
    taskInstance->latExpired();

    if (isGuard)
      return;

//  typedef smoc::SimulatorAPI::ChannelSinkInterface ChannelSinkInterface;
    typedef PossibleAction::PortOutInfo         PortOutInfo;

    if (PossibleAction *fr = taskInstance->getFiringRule()) {
      for (PortOutInfo const &portInfo : fr->getPortOutInfos()) {
        AbstractRoute *route = getAbstractRouteOfPort(portInfo.port);
        route->start<void>(portInfo.produced, nullptr, [](void *, size_t n, smoc::SimulatorAPI::ChannelSinkInterface *out) {
            out->commFinish(n);
          });
      }
    }
    delete taskInstance;
  }

} } // namespace SystemC_VPC::Detail
