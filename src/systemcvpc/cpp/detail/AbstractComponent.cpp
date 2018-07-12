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
#include <systemcvpc/Mappings.hpp>

#include "Director.hpp"
#include "AbstractComponent.hpp"
#include "ProcessControlBlock.hpp"
#include "ComponentObserver.hpp"
#include "ComponentInfo.hpp"
#include "HysteresisLocalGovernor.hpp"
#include "FastLink.hpp"

#include "dynload/dll.hpp"
#include "PluggablePowerGovernor.hpp"

#include "tracing/TracerIf.hpp"

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
    setPowerMode(translatePowerMode(powerMode));
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
  /// Handle interfaces for SystemC_VPC::ComponentModel
  ///

  void AbstractComponent::setPowerMode(const PowerMode* mode){
    this->powerMode = translatePowerMode(mode->getName());
    this->updatePowerConsumption();

    if(timingPools.find(powerMode) == timingPools.end()){
      timingPools[powerMode].reset(new FunctionTimingPool());
    }
    this->timingPool = timingPools[powerMode];
  }

  const PowerMode* AbstractComponent::getPowerMode() const {
    return this->powerMode;
  }

  ///
  /// Other stuff
  ///

  AbstractComponent::Factories AbstractComponent::factories;


  AbstractComponent::~AbstractComponent() {
    this->timingPools.clear();
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

      std::string powerMode = attPtr->getNextAttribute(i).first;
      const PowerMode *power = this->translatePowerMode(powerMode);

      if(powerTables.find(power) == powerTables.end()){
        powerTables[power] = PowerTable();
      }

      PowerTable &powerTable=powerTables[power];

      if(powerAtt->hasParameter("IDLE")){
        std::string v = powerAtt->getParameter("IDLE");
        const double value = atof(v.c_str());
        powerTable[ComponentState::IDLE] = value;
      }
      if(powerAtt->hasParameter("RUNNING")){
        std::string v = powerAtt->getParameter("RUNNING");
        const double value = atof(v.c_str());
        powerTable[ComponentState::RUNNING] = value;
      }
      if(powerAtt->hasParameter("STALLED")){
        std::string v = powerAtt->getParameter("STALLED");
        const double value = atof(v.c_str());
        powerTable[ComponentState::STALLED] = value;
      }
      if(powerAtt->hasParameter("transaction_delay")) {
        this->transactionDelays[power] =
          Director::createSC_Time(powerAtt->getParameter("transaction_delay"));
      }
      if(powerAtt->hasParameter("transfer_delay")) {
        this->transactionDelays[power] =
          Director::createSC_Time(powerAtt->getParameter("transfer_delay"));
      }

    }

    return true;
  }

  bool AbstractComponent::processMCG(AttributePtr attPtr)
  {
    // hierarchical format
    if(!attPtr->isType("multicastgroup")) {
      return false;
    }
    MultiCastGroup mcg = attPtr->getValue();

    for(size_t i=0; i<attPtr->getAttributeSize();++i){
         AttributePtr mcgAtt = attPtr->getNextAttribute(i).second;
         if(mcgAtt->isType("task")){
             std::string task = mcgAtt->getValue();
             ProcessId pid = Director::getInstance().getProcessId(task);
             multiCastGroups[pid] = mcg;
         }
    }
    return true;
  }


  /**
   *
   */
  bool AbstractComponent::setAttribute(AttributePtr attribute){
    if (processPower(attribute)) {
      return true;
    } else if (processMCG(attribute)) {
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

  FunctionTimingPtr AbstractComponent::getTiming(const PowerMode *mode, ProcessId pid) {
    if (timingPools.find(mode) == timingPools.end()) {
      timingPools[mode].reset(new FunctionTimingPool());
    }
    FunctionTimingPoolPtr pool = this->timingPools[mode];
    if (pool->find(pid) == pool->end()) {
      (*pool)[pid].reset(new FunctionTiming());
      (*pool)[pid]->setBaseDelay(this->transactionDelays[mode]);
      //sc_core::sc_time a = this->transactionDelays[mode];
      //std::cout << a;
    }
    return (*pool)[pid];
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

  AbstractComponent::MultiCastGroupInstance* AbstractComponent::getMultiCastGroupInstance(TaskInstance* actualTask){
    if(multiCastGroupInstances.size()!=0 ){
      //there are MultiCastGroupInstances, let's find the correct one
      for(std::list<MultiCastGroupInstance*>::iterator list_iter = multiCastGroupInstances.begin();
          list_iter != multiCastGroupInstances.end(); list_iter++)
      {
            MultiCastGroupInstance* mcgi = *list_iter;
          if(mcgi->mcg == multiCastGroups[actualTask->getProcessId()]){
            bool existing =  (mcgi->task->getProcessId() == actualTask->getProcessId());
            for(std::list<TaskInstance*>::iterator tasks_iter = mcgi->additional_tasks->begin();
                tasks_iter != mcgi->additional_tasks->end(); tasks_iter++){
                TaskInstance* task = *tasks_iter;
                if(task->getProcessId() == actualTask->getProcessId()){
                    existing = true;
                }
            }
            //we assume a fixed order of token-events, thus, the first free one is the correct one.
            if(!existing){
                mcgi->additional_tasks->push_back(actualTask);
                assert(mcgi->timestamp == sc_core::sc_time_stamp()); // if not, MultiCastMessage reached at different times...
                return mcgi;
            }
          }
      }
    }
    // no Instance found, create new one
    MultiCastGroupInstance* newInstance = new MultiCastGroupInstance();
    newInstance->mcg = multiCastGroups[actualTask->getProcessId()];
    newInstance->timestamp = sc_core::sc_time_stamp();
    newInstance->task = actualTask;
    newInstance->additional_tasks = new  std::list<TaskInstance*>();
    multiCastGroupInstances.push_back(newInstance);
    return newInstance;
  }

  AbstractComponent::AbstractComponent(std::string const &name)
    : sc_core::sc_module(sc_core::sc_module_name(name.c_str()))
    , Delayer(name)
    , requestExecuteTasks(false)
    , localGovernorFactory(nullptr)
    , midPowerGov(nullptr)
    , powerAttribute(new Attribute("",""))
    , powerMode(nullptr)
    , canExecuteTasks(true)
  {
    if(powerTables.find(getPowerMode()) == powerTables.end()){
      powerTables[getPowerMode()] = PowerTable();
    }

    PowerTable &powerTable=powerTables[getPowerMode()];
    powerTable[ComponentState::IDLE]    = 0.0;
    powerTable[ComponentState::RUNNING] = 1.0;
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

  void AbstractComponent::registerTask(TaskInterface *task) {
    // This should be the first time the actor appeared here.
    ProcessControlBlock *pcb = this->createPCB(task->name());
    pcb->setScheduledTask(task);
    pcb->configure(task->name(), true);
    task->setScheduler(this);
    task->setSchedulerInfo(pcb);
  }

  void AbstractComponent::registerFiringRule(TaskInterface *actor, smoc::SimulatorAPI::FiringRuleInterface *fr) {
    const char                        *actorName       = actor->name();
    smoc::SimulatorAPI::FunctionNames  actionNames     = fr->getActionNames();
    smoc::SimulatorAPI::FunctionNames  guardNames      = fr->getGuardNames();
    size_t                             guardComplexity = fr->getGuardComplexity();

    ProcessControlBlock       *pcb = static_cast<ProcessControlBlock *>(actor->getSchedulerInfo());
    ProcessId           const  pid = pcb->getPid();

    assert(Director::getInstance().getProcessId(actorName) == pid);
    assert(actor->getScheduler() == this);

    try {
      SystemC_VPC::VpcTask::Ptr vpcTask = SystemC_VPC::getCachedTask(static_cast<ScheduledTask &>(*actor));

      //TODO: SystemC_VPC::Timing -> Timing
      const SystemC_VPC::Components & components = SystemC_VPC::getComponents();

      for (SystemC_VPC::Components::value_type const &component_pair : components) {
        std::string            componentName = component_pair.first;
        SystemC_VPC::Component::Ptr component     = component_pair.second;

        if (SystemC_VPC::Mappings::isMapped(vpcTask, component)) {
          SystemC_VPC::TimingsProvider::Ptr provider = component->getTimingsProvider();
          pcb->setPriority(vpcTask->getPriority());  // GFR BUGFIX
          pcb->setTaskIsPSM(vpcTask->isPSM());
          if (provider->hasDefaultActorTiming(actorName)) {

            SystemC_VPC::functionTimingsPM timingsPM = provider->getActionTimings(actorName);

            for (SystemC_VPC::functionTimingsPM::iterator it=timingsPM.begin() ; it != timingsPM.end(); it++ ) {
              std::string powermode = (*it).first;
              pcb->setTiming(provider->getActionTiming(actorName,powermode));
            }

          }
          for (std::string const &guard : guardNames) {
            if (provider->hasGuardTimings(guard)) {

              SystemC_VPC::functionTimingsPM timingsPM = provider->getGuardTimings(guard);
              for (SystemC_VPC::functionTimingsPM::iterator it=timingsPM.begin() ; it != timingsPM.end(); it++ )
              {
                std::string powermode = (*it).first;
                pcb->setTiming(provider->getGuardTiming(guard,powermode));

                ConfigCheck::configureTiming(pid, guard);
              }
            }
          }
          for (std::string const &action : actionNames) {
            if (provider->hasActionTimings(action)) {

              SystemC_VPC::functionTimingsPM timingsPM = provider->getActionTimings(action);
              for (SystemC_VPC::functionTimingsPM::iterator it=timingsPM.begin() ; it != timingsPM.end(); it++ )
              {
                std::string powermode = (*it).first;
                pcb->setTiming(provider->getActionTiming(action,powermode));
                ConfigCheck::configureTiming(pid, action);
              }
            }
          }
        }
      }
    } catch (std::exception &e) {
      std::cerr << "Actor registration failed for \"" << actorName <<
          "\". Got exception:\n" << e.what() << std::endl;
      exit(-1);
    }

    FunctionIds     actionIds;
    FunctionIds     guardIds;

    for (FunctionNames::const_iterator iter = actionNames.begin();
         iter != actionNames .end();
         ++iter){
      Director::getInstance().debugFunctionNames[pid].insert(*iter);
      //check if we have timing data for this function in the XML configuration
      if (Director::getInstance().hasFunctionId(*iter)) {
        actionIds.push_back(Director::getInstance().getFunctionId(*iter) );
      }
      ConfigCheck::modelTiming(pid, *iter);
    }
    for (FunctionNames::const_iterator iter = guardNames.begin();
         iter != guardNames.end();
         ++iter){
      Director::getInstance().debugFunctionNames[pid].insert(*iter);
      if(Director::getInstance().hasFunctionId(*iter)){
        guardIds.push_back(Director::getInstance().getFunctionId(*iter) );
      }
      ConfigCheck::modelTiming(pid, *iter);
    }

    fr->setSchedulerInfo(new FastLink(this, pid, actionIds, guardIds, guardComplexity));
  }

  void AbstractComponent::checkFiringRule(TaskInterface *task, smoc::SimulatorAPI::FiringRuleInterface *fr) {
    FastLink *fLink = static_cast<FastLink *>(fr->getSchedulerInfo());

    ProcessControlBlock *pcb = getPCBOfTaskInterface(task);
    TaskInstance taskInstance(nullptr);
    taskInstance.setPCB(pcb);
    taskInstance.setProcessId(fLink->process);
    taskInstance.setFiringRule(fr);
    taskInstance.setName(task->name()+std::string("_check"));
    taskInstance.setFunctionIds(fLink->actionIds );

    FunctionTimingPtr timing =
        this->getTiming(this->getPowerMode(), fLink->process);

    if(!fLink->guardIds.empty())
      taskInstance.setDelay(
          timing->getDelay(fLink->guardIds) +
          fLink->complexity * sc_core::sc_time(0.1, sc_core::SC_NS));
    else
      taskInstance.setDelay(
          fLink->complexity * sc_core::sc_time(0.1, sc_core::SC_NS));
    taskInstance.setRemainingDelay(taskInstance.getDelay());
    taskInstance.setLatency(taskInstance.getDelay());
    this->check(&taskInstance);
  }

  class AbstractComponent::InputsAvailableListener
    : public CoSupport::SystemC::EventListener
  {
    typedef CoSupport::SystemC::EventWaiter
        EventWaiter;
  public:
    InputsAvailableListener(
        TaskInterface                           *task,
        smoc::SimulatorAPI::FiringRuleInterface *fr)
      : task(task), fr(fr)
      // We start with one to ensure that signaled does not call compute before wait is called!
      , missing(1)
      {}

    VPCEvent::Ptr acquireEvent() {
      VPCEvent::Ptr retval(new VPCEvent());
      retval->addListener(this);
      ++missing;
      return retval;
    }

    void wait() {
      if (!--missing)
        compute();
    }
  private:
    ~InputsAvailableListener() {}

    void signaled(EventWaiter *e) {
      assert(*e);
      e->delListener(this);
      if (!--missing)
        compute();
    }

    // The lifetime of the given EventWaiter is over
    void eventDestroyed(EventWaiter *e)
      { assert(!"WTF"); }

    // May be called when Event is active
    void renotified(EventWaiter *e)
      { assert(!"WTF"); }

    TaskInterface                           *task;
    smoc::SimulatorAPI::FiringRuleInterface *fr;
    size_t                                   missing;

    void compute() {
      FastLink            *fLink = static_cast<FastLink *>(fr->getSchedulerInfo());
      AbstractComponent   *comp  = static_cast<AbstractComponent *>(task->getScheduler());
      ProcessControlBlock *pcb   = getPCBOfTaskInterface(task);

      TaskInstance *taskInstance = new TaskInstance(nullptr);

      assert(pcb != NULL);

      FunctionTimingPtr timing =
          comp->getTiming(comp->getPowerMode(), fLink->process);

      //ugly hack: to make the random timing work correctly getDelay has to be called before getLateny, see Processcontrollbock.cpp for more information
      // Initialize with DII
      taskInstance->setDelay(timing->getDelay(fLink->actionIds));
      // Initialize with DII
      taskInstance->setRemainingDelay(taskInstance->getDelay());
      // Initialize with Latency
      taskInstance->setLatency(timing->getLatency(fLink->actionIds));

      taskInstance->setPCB(pcb);
      taskInstance->setProcessId(fLink->process);
      taskInstance->setFiringRule(fr);
      taskInstance->setName(task->name());
      taskInstance->setFunctionIds(fLink->actionIds );
      comp->compute(taskInstance);
      delete this;
    }
  };

  void AbstractComponent::executeFiringRule(TaskInterface *task, smoc::SimulatorAPI::FiringRuleInterface *fr) {

    typedef smoc::SimulatorAPI::FiringRuleInterface     FiringRuleInterface;
    typedef FiringRuleInterface::PortInInfo             PortInInfo;
    typedef FiringRuleInterface::PortOutInfo            PortOutInfo;

    InputsAvailableListener *ial = new InputsAvailableListener(task, fr);

    for (PortInInfo const &portInfo : fr->getPortInInfos()) {
      portInfo.port.commStart(portInfo.consumed);
      FastLink  *fLink = static_cast<FastLink *>(portInfo.port.getSchedulerInfo());
      fLink->read(portInfo.required, EventPair(nullptr, ial->acquireEvent()));
    }
    for (PortOutInfo const &portInfo : fr->getPortOutInfos()) {
      portInfo.port.commStart(portInfo.produced);
    }
    // Wait for arrival of all inputs, i.e., the routing delay.
    ial->wait();
  }

  TaskInstance *AbstractComponent::executeHop(ProcessControlBlock *pcb, size_t quantum, EventPair const &np) {
    TaskInstance *taskInstance = new TaskInstance(nullptr);

    assert(pcb != NULL);

    FunctionTimingPtr timing =
        this->getTiming(this->getPowerMode(), pcb->getPid());

    FunctionIds fids; // empty functionIds
    fids.push_back(Director::getInstance().getFunctionId("1"));

    //ugly hack: to make the random timing work correctly getDelay has to be called before getLateny, see Processcontrollbock.cpp for more information
    // Initialize with DII
    taskInstance->setDelay(quantum * timing->getDelay(fids));
    // Initialize with DII
    taskInstance->setRemainingDelay(taskInstance->getDelay());
    // Initialize with Latency
    taskInstance->setLatency(quantum * timing->getLatency(fids));

    taskInstance->setPCB(pcb);
    taskInstance->setProcessId(pcb->getPid());
    taskInstance->setName(pcb->getName());
    taskInstance->setFunctionIds(fids);
    taskInstance->setTimingScale(quantum);
    taskInstance->setBlockEvent(np);
    this->compute(taskInstance);
    return taskInstance;
  }

  /// Called once per actor firing to indicate that the DII of the task instance is over.
  void AbstractComponent::finishDiiTaskInstance(TaskInstance *taskInstance) {
    this->Tracing::TraceableComponent::finishDiiTaskInstance(taskInstance);
    if (taskInstance->getBlockEvent().dii.get())
      taskInstance->getBlockEvent().dii->notify();

    typedef smoc::SimulatorAPI::FiringRuleInterface FiringRuleInterface;
    typedef FiringRuleInterface::PortInInfo         PortInInfo;

    if (FiringRuleInterface *fr = taskInstance->getFiringRule()) {
      for (PortInInfo const &portInfo : fr->getPortInInfos()) {
        portInfo.port.getSource()->commFinish(portInfo.consumed);
      }
    }
  }

  class AbstractComponent::OutputWrittenListener
    : public CoSupport::SystemC::EventListener
  {
    typedef CoSupport::SystemC::EventWaiter
        EventWaiter;
  public:
    OutputWrittenListener(
        smoc::SimulatorAPI::ChannelSinkInterface *sink,
        size_t                                    produced)
      : sink(sink), produced(produced)
      // We start with one to ensure that signaled does not call write before wait is called!
      , missing(1)
      , dropped(false)
      {}

    VPCEvent::Ptr acquireEvent() {
      VPCEvent::Ptr retval(new VPCEvent());
      retval->addListener(this);
      ++missing;
      return retval;
    }

    void wait() {
      if (!--missing)
        write();
    }
  private:
    ~OutputWrittenListener() {}

    void signaled(EventWaiter *e) {
      assert(*e);
      e->delListener(this);
      dropped = static_cast<VPCEvent *>(e)->getDropped();
      if (!--missing)
        write();
    }

    // The lifetime of the given EventWaiter is over
    void eventDestroyed(EventWaiter *e)
      { assert(!"WTF"); }

    // May be called when Event is active
    void renotified(EventWaiter *e)
      { assert(!"WTF"); }

    smoc::SimulatorAPI::ChannelSinkInterface *sink;
    size_t                                    produced;
    size_t                                    missing;
    bool                                      dropped;

    void write() {
      sink->commFinish(produced, dropped);
      delete this;
    }
  };


  /// Called once per actor firing to indicate that the latency of the task instance is over.
  void AbstractComponent::finishLatencyTaskInstance(TaskInstance *taskInstance) {
    this->Tracing::TraceableComponent::finishLatencyTaskInstance(taskInstance);
    if (taskInstance->getBlockEvent().latency.get())
      taskInstance->getBlockEvent().latency->notify();

    typedef smoc::SimulatorAPI::FiringRuleInterface  FiringRuleInterface;
//  typedef smoc::SimulatorAPI::ChannelSinkInterface ChannelSinkInterface;
    typedef FiringRuleInterface::PortOutInfo         PortOutInfo;

    if (FiringRuleInterface *fr = taskInstance->getFiringRule()) {
      for (PortOutInfo const &portInfo : fr->getPortOutInfos()) {
        // FIXME: Multicast
        assert(portInfo.port.getSinks().size() == 1);
        OutputWrittenListener *owl = new OutputWrittenListener(
            portInfo.port.getSinks().front(), portInfo.produced);
        FastLink  *fLink = static_cast<FastLink *>(portInfo.port.getSchedulerInfo());
        fLink->write(portInfo.produced, EventPair(nullptr, owl->acquireEvent()));
        owl->wait();
//      for (ChannelSinkInterface *sink : portInfo.port.getSinks()) {
//        sink->commFinish(portInfo.produced);
//      }
      }
    }
    // Remember last acknowledged task time
    Director::getInstance().end = sc_core::sc_time_stamp();
    taskInstance->release();
  }

} } // namespace SystemC_VPC::Detail
