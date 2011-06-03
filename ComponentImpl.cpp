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

#include <systemcvpc/vpc_config.h>

#include <systemcvpc/ComponentImpl.hpp>
#include <systemcvpc/Scheduler.hpp>
#include <systemcvpc/FCFSScheduler.hpp>
#include <systemcvpc/TDMAScheduler.hpp>
#include <systemcvpc/FlexRayScheduler.hpp>
#include <systemcvpc/AVBScheduler.hpp>
#include <systemcvpc/RoundRobinScheduler.hpp>
#include <systemcvpc/PriorityScheduler.hpp>
#include <systemcvpc/RateMonotonicScheduler.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/TimeTriggeredCCScheduler.hpp>
#include <systemcvpc/PrioritySchedulerNoPreempt.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/PluggablePowerGovernor.hpp>
#include <systemcvpc/HysteresisLocalGovernor.hpp>
#include <systemcvpc/Task.hpp>
#include <systemcvpc/config/Component.hpp>

#include <float.h>

#include <systemcvpc/debug_config.hpp>
// if compiled with DBG_COMPONENT create stream and include debug macros
#ifdef DBG_COMPONENT
#include <CoSupport/Streams/DebugOStream.hpp>
#include <CoSupport/Streams/FilterOStream.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include <systemcvpc/debug_on.hpp>
#else
  #include <systemcvpc/debug_off.hpp>
#endif

namespace SystemC_VPC{

  /**
   *
   */
  void Component::schedule_thread(){
    sc_time timeslice;
    sc_time actualRemainingDelay;
    sc_time *overhead = new sc_time( SC_ZERO_TIME );
    int actualRunningIID;
    bool newTaskDuringOverhead=false;
    //wait(SC_ZERO_TIME);

    scheduler->initialize();
    fireStateChanged(ComponentState::IDLE);
    
    while(1){
      //determine the time slice for next scheduling descission and wait for
      bool hasTimeSlice= scheduler->getSchedulerTimeSlice( timeslice,
                                                           readyTasks,
                                                           runningTasks );
      startTime = sc_time_stamp();
      if(!newTaskDuringOverhead){ 
        if(runningTasks.size()<=0){                    // no running task
          if(hasTimeSlice){                           
            wait( timeslice - (*overhead),
                  notify_scheduler_thread ); 
          }else{
            wait( notify_scheduler_thread );
          }
        }else{                                        // a task allready runs
          if(hasTimeSlice && (timeslice - (*overhead)) < actualRemainingDelay){
            wait( timeslice - (*overhead),
                  notify_scheduler_thread );
          }else{
            wait( actualRemainingDelay,
                  notify_scheduler_thread );
          }
          sc_time runTime=sc_time_stamp()-startTime;
          assert(runTime.value()>=0);
          actualRemainingDelay-=runTime;

          assert(actualRemainingDelay.value()>=0);

          DBG_OUT("Component " << this->getName()
                    << "> actualRemainingDelay= "
                    << actualRemainingDelay.value() << " for iid="
                    << actualRunningIID << " at: "
                    << sc_time_stamp().to_default_time_units()
                    << std::endl);

          if(actualRemainingDelay.value()==0){
            // all execution time simulated -> BLOCK running task.
            Task *task=runningTasks[actualRunningIID];

          DBG_OUT(this->getName() << " IID: " << actualRunningIID<< " > ");
          DBG_OUT(this->getName() << " removed Task: " << task->getName()
                  << " at: " << sc_time_stamp().to_default_time_units()
                  << std::endl);

            //notify(*(task->blockEvent));
            scheduler->removedTask(task);
            fireStateChanged(ComponentState::IDLE);
            task->traceFinishTaskDii();
            runningTasks.erase(actualRunningIID);

            task->getBlockEvent().dii->notify();
            moveToRemainingPipelineStages(task);
            //wait(SC_ZERO_TIME);
          }else{

            // store remainingDelay
            runningTasks[actualRunningIID]->setRemainingDelay(
              actualRemainingDelay );
          }
        }
      }else{
        newTaskDuringOverhead=false;
      }

      this->addTasks();

      int taskToResign,taskToAssign;
      scheduling_decision decision =
        scheduler->schedulingDecision(taskToResign,
                                      taskToAssign,
                                      readyTasks,
                                      runningTasks);

      //resign task
      if(decision==RESIGNED || decision==PREEMPT){
        readyTasks[taskToResign]=runningTasks[taskToResign];
        runningTasks.erase(taskToResign);
        actualRunningIID=-1;
        readyTasks[taskToResign]->setRemainingDelay(actualRemainingDelay);
        fireStateChanged(ComponentState::IDLE);
        readyTasks[taskToResign]->traceResignTask();
      }

      sc_time timestamp=sc_time_stamp();
      if( overhead != NULL ) delete overhead;
      overhead=scheduler->schedulingOverhead();

      if( overhead != NULL ){
#ifndef NO_VCD_TRACES
        schedulerTrace =  SystemC_VPC::Tracing::S_RUNNING;
#endif //NO_VCD_TRACES
        //    actual time    < endtime
        while( (sc_time_stamp() < timestamp + (*overhead)) ){ 

          wait( (timestamp+(*overhead))-sc_time_stamp(),
                notify_scheduler_thread );

        }

        // true if some task becames ready during overhead waiting
        newTaskDuringOverhead=(newTasks.size()>0);
#ifndef NO_VCD_TRACES
        schedulerTrace = SystemC_VPC::Tracing::S_READY;
#endif //NO_VCD_TRACES
      }else {
        // avoid failures
        overhead=new sc_time(SC_ZERO_TIME);
      }


      //assign task
      if(decision==ONLY_ASSIGN || decision==PREEMPT){
        runningTasks[taskToAssign]=readyTasks[taskToAssign];
        runningTasks[taskToAssign]->traceAssignTask();
        readyTasks.erase(taskToAssign);
        actualRunningIID=taskToAssign;
        DBG_OUT("IID: " << taskToAssign << "> remaining delay for "
             << runningTasks[taskToAssign]->getName());
        actualRemainingDelay 
          = sc_time(runningTasks[taskToAssign]->getRemainingDelay());
        DBG_OUT(" is " << runningTasks[taskToAssign]->getRemainingDelay()
             << endl);
        fireStateChanged(ComponentState::RUNNING);

        /* */
        Task * assignedTask = runningTasks[taskToAssign];
        if(assignedTask->isBlocking() /* && !assignedTask->isExec() */) {
          blockMutex++;
          if(blockMutex == 1) {
            DBG_OUT(this->getName() << " scheduled blocking task: "
                    << assignedTask->getName() << std::endl);
            assignedTask->ackBlockingCompute();
            DBG_OUT(this->getName() << " enter wait: " << std::endl);
            fireStateChanged(ComponentState::STALLED);
            assignedTask->traceBlockTask();
            while(!assignedTask->isExec()){
              blockCompute.reset();
              CoSupport::SystemC::wait(blockCompute);
              this->addTasks();
            }
            DBG_OUT(this->getName() << " exit wait: " << std::endl);
            fireStateChanged(ComponentState::RUNNING);
            assignedTask->traceAssignTask();
            if(assignedTask->isBlocking()){
              DBG_OUT(this->getName() << " exec Task: "
                      << assignedTask->getName() << " @  " << sc_time_stamp()
                      << std::endl);
              // task is still blocking: exec task
            } else {
              DBG_OUT(this->getName() << " abort Task: "
                      << assignedTask->getName() << " @  " << sc_time_stamp()
                      << std::endl);

              //notify(*(task->blockEvent));
              scheduler->removedTask(assignedTask);
              fireStateChanged(ComponentState::IDLE);
              assignedTask->traceFinishTaskDii();
              //FIXME: notify latency ??
              //assignedTask->traceFinishTaskLatency();
              runningTasks.erase(actualRunningIID);
             
            }
          }else{
            assert(blockMutex>1);
            scheduler->removedTask(assignedTask);
            fireStateChanged(ComponentState::IDLE);
            assignedTask->traceFinishTaskDii();
            //FIXME: notify latency ??
            //assignedTask->traceFinishTaskLatency();
            runningTasks.erase(actualRunningIID);
            assignedTask->abortBlockingCompute();
          }
          blockMutex--;
        }
        /* */
      }
    }

  }


  /**
   *
   */
  void Component::setScheduler(Config::Component::Ptr component)
  {
    Config::Scheduler::Type type = component->getScheduler();
    switch (type) {
      case Config::Scheduler::RoundRobin:
        scheduler = new RoundRobinScheduler();
        break;
      case Config::Scheduler::StaticPriority_NP:
        scheduler = new PrioritySchedulerNoPreempt();
      case Config::Scheduler::StaticPriority_P:
        scheduler = new PriorityScheduler();
        break;
      case Config::Scheduler::RateMonotonic:
        scheduler = new RateMonotonicScheduler();
        break;
      case Config::Scheduler::FCFS:
        scheduler = new FCFSScheduler();
        break;
      case Config::Scheduler::TDMA:
        scheduler = new TDMAScheduler();
        break;
      case Config::Scheduler::FlexRay:
        scheduler = new FlexRayScheduler();
        break;
      case Config::Scheduler::AVB:
        scheduler = new AVBScheduler();
        break;
      case Config::Scheduler::TTCC:
        scheduler = new TimeTriggeredCCScheduler();
      default:
        scheduler = new FCFSScheduler();
    }
  }

  /**
   *
   */
  void Component::compute(Task* actualTask){

    /* * /
    if(blockMutex > 0) {
      actualTask->abortBlockingCompute();
      return;
    }
    / * */

    ProcessId pid = actualTask->getProcessId();
    ProcessControlBlockPtr pcb = this->getPCB(pid);
    actualTask->setPCB(pcb);
    actualTask->setTiming(this->getTiming(this->getPowerMode(), pid));

    DBG_OUT(this->name() << "->compute ( " << actualTask->getName()
            << " ) at time: " << sc_time_stamp()
            << " mode: " << this->getPowerMode()->getName()
            << std::endl);

    // reset the execution delay
    actualTask->initDelays();
    DBG_OUT("dii: " << actualTask->getRemainingDelay() << std::endl);
    DBG_OUT("latency: " << actualTask->getLatency()  << std::endl);
    
    //store added task
    newTasks.push_back(actualTask);

    //awake scheduler thread
    notify_scheduler_thread.notify();
    blockCompute.notify();
  }



  /**
   *
   */
  void Component::requestBlockingCompute(Task* task, RefCountEventPtr blocker){
    task->setExec(false);
    task->setBlockingCompute( blocker );
    this->compute( task );
  }

  /**
   *
   */
  void Component::execBlockingCompute(Task* task, RefCountEventPtr blocker){
    task->setExec(true);
    blockCompute.notify();
  }


  /**
   *
   */
  void Component::abortBlockingCompute(Task* task, RefCountEventPtr blocker){
    task->resetBlockingCompute();
    blockCompute.notify();
  }

  /**
   *
   */
  void Component::remainingPipelineStages(){
    while(1){
      if(pqueue.size() == 0){
        wait(remainingPipelineStages_WakeUp);
      }else{
        timePcbPair front = pqueue.top();

        //cerr << "Pop from list: " << front.time << " : " 
        //<< front.pcb->getBlockEvent().latency << endl;
        sc_time waitFor = front.time-sc_time_stamp();
        assert(front.time >= sc_time_stamp());
        //cerr << "Pipeline> Wait till " << front.time
        //<< " (" << waitFor << ") at: " << sc_time_stamp() << endl;
        wait( waitFor, remainingPipelineStages_WakeUp );

        sc_time rest = front.time-sc_time_stamp();
        assert(rest >= SC_ZERO_TIME);
        if(rest > SC_ZERO_TIME){
          //cerr << "------------------------------" << endl;
        }else{
          assert(rest == SC_ZERO_TIME);
          //cerr << "Ready! releasing task (" <<  front.time <<") at: "
          //<< sc_time_stamp() << endl;

          front.task->traceFinishTaskLatency();

          // Latency over -> remove Task
          Director::getInstance().signalLatencyEvent(front.task);

          //wait(SC_ZERO_TIME);
          pqueue.pop();
        }
      }

    }
  }

  /**
   *
   */
  void Component::moveToRemainingPipelineStages(Task* task){
    sc_time now                 = sc_time_stamp();
    sc_time restOfLatency       = task->getLatency()  - task->getDelay();
    sc_time end                 = now + restOfLatency;
    if(end <= now){
      //early exit if (Latency-DII) <= 0
      //std::cerr << "Early exit: " << task->getName() << std::endl;
      task->traceFinishTaskLatency();
      Director::getInstance().signalLatencyEvent(task);
      return;
    }
    timePcbPair pair;
    pair.time = end;
    pair.task  = task;
    //std::cerr << "Rest of pipeline added: " << task->getName()
    //<< " (EndTime: " << pair.time << ") " << std::endl;
    pqueue.push(pair);
    remainingPipelineStages_WakeUp.notify();
  }

  void Component::updatePowerConsumption()
  {
    this->setPowerConsumption(powerTables[getPowerMode()][getComponentState()]);
    // Notify observers (e.g. powersum)
    this->fireNotification(this);
  }

  void Component::fireStateChanged(const ComponentState &state)
  {
    this->setComponentState(state);
    this->updatePowerConsumption();
  }

  void Component::addTasks(){
    //look for new tasks (they called compute)
    while(newTasks.size()>0){
      Task *newTask;
      newTask=newTasks.front();
      newTasks.pop_front();
      DBG_OUT(this->getName() << " received new Task: "
              << newTask->getName() << " at: "
              << sc_time_stamp().to_default_time_units() << std::endl);
      newTask->traceReleaseTask();
      //insert new task in read list
      assert( readyTasks.find(newTask->getInstanceId())   == readyTasks.end()
              /* A task can call compute only one time! */);
      assert( runningTasks.find(newTask->getInstanceId()) ==
              runningTasks.end()
              /* A task can call compute only one time! */);

      readyTasks[newTask->getInstanceId()]=newTask;
      scheduler->addedNewTask(newTask);
    }

  }

  void Component::initialize(const Director* d){
    //std::cerr << "Component::initialize" << std::endl;
    if(powerAttribute->isType("")){
      //std::cerr << "disabled local power governor" << std::endl;
      return;
    }

    if(NULL == localGovernorFactory){
      localGovernorFactory = new InternalLoadHysteresisGovernorFactory();
    }

    // governor parameter
    localGovernorFactory->processAttributes(powerAttribute);

    //create local governor
    midPowerGov=localGovernorFactory->createPlugIn();
    midPowerGov->setGlobalGovernor(d->topPowerGov);
    this->addPowerGovernor(midPowerGov);
    
  }

  bool Component::setAttribute(AttributePtr attribute){
    bool isComponentAttribute = AbstractComponent::setAttribute(attribute);
    if (!isComponentAttribute){
      scheduler->setAttribute(attribute);
    }
    return true;
  }

  Component::~Component(){
    this->setPowerConsumption(0.0);
    this->fireNotification(this);
#ifndef NO_POWER_SUM
    this->removeObserver(powerSumming);
    delete powerSumming;
    delete powerSumStream;
#endif // NO_POWER_SUM
  }
} //namespace SystemC_VPC
