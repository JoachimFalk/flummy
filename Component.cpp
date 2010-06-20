/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * Component.cpp
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/
#include <systemcvpc/Component.hpp>
#include <systemcvpc/Scheduler.hpp>
#include <systemcvpc/FCFSScheduler.hpp>
#include <systemcvpc/TDMAScheduler.hpp>
#include <systemcvpc/FlexRayScheduler.hpp>
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
#ifndef NO_VCD_TRACES
            if(task->getTraceSignal()!=0)
              task->getTraceSignal()->traceSleeping();
#endif //NO_VCD_TRACES
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
#ifndef NO_VCD_TRACES
        if(readyTasks[taskToResign]->getTraceSignal()!=0)
          readyTasks[taskToResign]->getTraceSignal()->traceReady();
#endif //NO_VCD_TRACES
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
#ifndef NO_VCD_TRACES
        if(runningTasks[taskToAssign]->getTraceSignal()!=0)
          runningTasks[taskToAssign]->getTraceSignal()->traceRunning();
#endif //NO_VCD_TRACES
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
#ifndef NO_VCD_TRACES
            if(assignedTask->getTraceSignal()!=0)
              assignedTask->getTraceSignal()->traceBlocking();
#endif //NO_VCD_TRACES
            while(!assignedTask->isExec()){
              blockCompute.reset();
              CoSupport::SystemC::wait(blockCompute);
              this->addTasks();
            }
            DBG_OUT(this->getName() << " exit wait: " << std::endl);
            fireStateChanged(ComponentState::RUNNING);
#ifndef NO_VCD_TRACES
            if(assignedTask->getTraceSignal()!=0)
              assignedTask->getTraceSignal()->traceRunning();
#endif //NO_VCD_TRACES
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
#ifndef NO_VCD_TRACES
              if(assignedTask->getTraceSignal()!=0)
                assignedTask->getTraceSignal()->traceSleeping();
#endif //NO_VCD_TRACES
              runningTasks.erase(actualRunningIID);
             
            }
          }else{
            assert(blockMutex>1);
            scheduler->removedTask(assignedTask);
            fireStateChanged(ComponentState::IDLE);
#ifndef NO_VCD_TRACES
            if(assignedTask->getTraceSignal()!=0)
              assignedTask->getTraceSignal()->traceSleeping();
#endif //NO_VCD_TRACES
            runningTasks.erase(actualRunningIID);
            assignedTask->abortBlockingCompute();
          }
          blockMutex--;
        }
        /* */
      }
    }

  }


  bool Component::processPower(AttributePtr attPtr)
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

    }
        
    return true;
  }

  /**
   *
   */
  void Component::setAttribute(AttributePtr attribute){
    if(processPower(attribute)){
      return;
    }

    if(attribute->isType("transaction_delay")) {
      this->transactionDelays[this->getPowerMode()] =
        Director::createSC_Time(attribute->getValue());
      return;
    }
    
    if(attribute->isType("transaction")) {
      unsigned int transactionSize = 1;
      sc_time transactionDelay     = SC_ZERO_TIME;
      if(attribute->hasParameter("delay")){
        transactionDelay =
          Director::createSC_Time(attribute->getParameter("delay"));
      }

      if(attribute->hasParameter("size")){
        transactionSize = atoi(attribute->getParameter("size").c_str());
      }

      this->transactionDelays[this->getPowerMode()] = transactionDelay;
      // FIXME: add transactionSize
      return;
    }
    
    scheduler->setAttribute(attribute);
  }

  /**
   *
   */
  void  Component::setScheduler(const char *schedulername){
    if( 0==strncmp(schedulername,STR_ROUNDROBIN,strlen(STR_ROUNDROBIN))
        || 0==strncmp(schedulername,STR_RR,strlen(STR_RR)) ){
      scheduler=new RoundRobinScheduler((const char*)schedulername);
      }else if( 0==strncmp(schedulername,
                         STR_PRIORITYSCHEDULERNOPREEMPT,strlen(STR_PRIORITYSCHEDULERNOPREEMPT))
              || 0==strncmp(schedulername,STR_PSNOPRE,strlen(STR_PSNOPRE)) ){
      scheduler=new PrioritySchedulerNoPreempt((const char*)schedulername);
    }else if( 0==strncmp(schedulername,
                         STR_PRIORITYSCHEDULER,strlen(STR_PRIORITYSCHEDULER))
              || 0==strncmp(schedulername,STR_PS,strlen(STR_PS)) ){
      scheduler=new PriorityScheduler((const char*)schedulername);

    }else if( 0==strncmp(schedulername,STR_RATEMONOTONIC,
                         strlen(STR_RATEMONOTONIC))
              || 0==strncmp(schedulername,STR_RM,strlen(STR_RM))){
      scheduler=new RateMonotonicScheduler((const char*)schedulername);

    }else if( 0==strncmp(schedulername,STR_FIRSTCOMEFIRSTSERVE,
                         strlen(STR_FIRSTCOMEFIRSTSERVE))
              || 0==strncmp(schedulername,STR_FCFS,strlen(STR_FCFS))){
      scheduler=new FCFSScheduler();
      
      // TDMA hat nur einen Namen ;-)
    }else if( 0==strncmp(schedulername,STR_TDMA,
                         strlen(STR_TDMA))){
      scheduler=new TDMAScheduler((const char*)schedulername);

    }else if( 0==strncmp(schedulername,STR_FLEXRAY,
                         strlen(STR_FLEXRAY))){
      scheduler=new FlexRayScheduler((const char*)schedulername);
    }else if( 0==strncmp(schedulername,STR_TTCC,
                         strlen(STR_TTCC))){
      scheduler=new TimeTriggeredCCScheduler((const char*)schedulername);
    }else{
      //    cerr << "Scheduler: "<< STR_FIRSTCOMEFIRSTSERVE << endl;
      scheduler=new FCFSScheduler();
    }
  }

  /**
   *
   */
  void Component::informAboutMapping(std::string module){
#ifndef NO_VCD_TRACES
    Tracing *newsignal = new Tracing(getName(), module);
    trace_map_by_name.insert(std::pair<std::string,Tracing* >(module, newsignal));
    sc_trace(this->traceFile, *newsignal->traceSignal, module.c_str());
    newsignal->traceSleeping();
#endif //NO_VCD_TRACES

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
    PCBPool &pool = this->getPCBPool();
    assert(pool.find(pid) != pool.end());
    ProcessControlBlock* pcb = pool[pid];
    actualTask->setPCB(pcb);
    actualTask->setTiming(this->getTiming(this->getPowerMode(), pid));

    DBG_OUT(this->name() << "->compute ( " << actualTask->getName()
            << " ) at time: " << sc_time_stamp()
            << " mode: " << this->getPowerMode()->getName()
            << std::endl);

    // reset the execution delay
    actualTask->initDelays();
    DBG_OUT("Using " << actualTask->getRemainingDelay()
         << " as delay for function " << actualTask->getFunctionId() << "!"
         << std::endl);
    DBG_OUT("And " << actualTask->getLatency() << " as latency for function "
         << actualTask->getFunctionId() << "!" << std::endl);
    
#ifndef NO_VCD_TRACES
    {
      std::map<std::string, Tracing* >::iterator iter
        = trace_map_by_name.find(actualTask->getName());
      if( iter != trace_map_by_name.end() ){
        actualTask->setTraceSignal(iter->second);
      }
    }
#endif //NO_VCD_TRACES

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

          // Latency over -> remove Task
          this->notifyParentController(front.task);

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
      this->notifyParentController(task);
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
#ifndef NO_VCD_TRACES
      if(newTask->getTraceSignal()!=0)
        newTask->getTraceSignal()->traceReady();
#endif //NO_VCD_TRACES
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

  void Component::loadLocalGovernorPlugin(std::string plugin){
    //std::cerr << "Component::loadLocalGovernorPlugin" << std::endl;

    if(plugin == "") return;

    if(Component::factories.find(plugin) == Component::factories.end()){
      Component::factories[plugin] =
        new DLLFactory<PlugInFactory<PluggableLocalPowerGovernor> >
          (plugin.c_str());
    }

    localGovernorFactory = Component::factories[plugin]->factory;
  }

  Component::~Component(){
    this->setPowerConsumption(0.0);
    this->fireNotification(this);
#ifndef NO_POWER_SUM
    this->removeObserver(powerSumming);
    delete powerSumming;
    delete powerSumStream;
#endif // NO_POWER_SUM
#ifndef NO_VCD_TRACES
    for(std::map<std::string, Tracing* >::iterator iter
          = trace_map_by_name.begin();
        iter != trace_map_by_name.end();
        ++iter){
      delete iter->second;
    }
    trace_map_by_name.clear();
    sc_close_vcd_trace_file(traceFile);
#endif //NO_VCD_TRACES      
  }


  Component::Factories Component::factories;
} //namespace SystemC_VPC
