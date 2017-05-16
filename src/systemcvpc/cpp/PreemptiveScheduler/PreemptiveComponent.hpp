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

#ifndef HSCD_VPC_COMPONENT_H
#define HSCD_VPC_COMPONENT_H
#include <PreemptiveScheduler/Scheduler.hpp>
#include <systemc.h>

#include <systemcvpc/AbstractComponent.hpp>
#include <systemcvpc/ComponentInfo.hpp>
#include <systemcvpc/config/Scheduler.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/PowerMode.hpp>
#include <systemcvpc/PowerSumming.hpp>
#include <systemcvpc/vpc_config.h>
#include <systemcvpc/timetriggered/tt_support.hpp>

#include <vector>
#include <map>
#include <deque>
#include <queue>

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
   * \brief An implementation of AbstractComponent.
   * 
   */
  class PreemptiveComponent : public AbstractComponent{
    
    SC_HAS_PROCESS(PreemptiveComponent);

  public:

    /**
     * implementation of AbstractComponent::compute(ProcessControlBlock*)
     */
    virtual void compute(Task* task);

    /**
     *
     */
    bool setAttribute(AttributePtr attribute);

    /**
     *
     */
    virtual void requestBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker);
    
    /**
     *
     */
    virtual void execBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker);
    
    /**
     *
     */
    virtual void abortBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker);
    
    /**
     *
     */
    virtual void updatePowerConsumption();
      
    /**
     * \brief An implementation of AbstractComponent.
     */
    PreemptiveComponent( Config::Component::Ptr component)
      : AbstractComponent(component),
        blockMutex(0), max_used_buffer(0), max_avail_buffer(0)
    {
      SC_THREAD(schedule_thread);
      SC_THREAD(remainingPipelineStages);
      SC_METHOD(releaseActorsMethod);
      dont_initialize();
      sensitive << releaseActors;
      this->setPowerMode(this->translatePowerMode("SLOW"));
      setScheduler(component);

#ifndef NO_POWER_SUM
      std::string powerSumFileName(this->getName());
      powerSumFileName += ".dat";

      powerSumStream = new std::ofstream(powerSumFileName.c_str());
      powerSumming   = new PowerSumming(*powerSumStream);
      this->addObserver(powerSumming);
#endif // NO_POWER_SUM
    }
      
    virtual ~PreemptiveComponent();
    
    void addPowerGovernor(PluggableLocalPowerGovernor * gov){
      this->addObserver(gov);
    }

    void reactivateExecution();

    void notifyActivation(ScheduledTask * scheduledTask,
        bool active);

    bool addStream(ProcessId pid);

    bool closeStream(ProcessId pid);

  protected:
    virtual void schedule_thread() = 0;
    virtual void remainingPipelineStages() = 0;
    virtual void moveToRemainingPipelineStages(Task* task) = 0;

    sc_event remainingPipelineStages_WakeUp;
    std::priority_queue<timePcbPair> pqueue;
    bool pendingTask;

    Scheduler *scheduler;
    std::deque<Task*>      newTasks;
    std::deque<Task*>      disabledTasks;
    sc_event notify_scheduler_thread;
    Event blockCompute;
    size_t   blockMutex;
    unsigned int max_used_buffer;
    unsigned int max_avail_buffer;

    // time last task started
    sc_time startTime;

    void fireStateChanged(const ComponentState &state);

  private:
    sc_event releaseActors;
    TT::TimedQueue ttReleaseQueue;

#ifndef NO_POWER_SUM
    std::ofstream *powerSumStream;
    PowerSumming  *powerSumming;
#endif // NO_POWER_SUM

    void initialize(const Director* d);

    void setScheduler(Config::Component::Ptr component);

    void releaseActorsMethod();
  };

  template<class TASKTRACER>
  class ComponentImpl : public PreemptiveComponent{
  public:
    /**
     *
     */
    ComponentImpl( Config::Component::Ptr component)
      : PreemptiveComponent(component), taskTracer_(component){}

    /**
     *
     */
    virtual ~ComponentImpl() {}

    /**
     *
     */
    virtual Trace::Tracing * getOrCreateTraceSignal(std::string name)
    {
      return taskTracer_.getOrCreateTraceSignal(name);
    }

    /**
     *
     */
    void addTasks(){
      if(!this->getCanExecuteTasks()){
        std::deque<Task*>::iterator iter = newTasks.begin();
        for(;newTasks.size() > 0 && iter!=newTasks.end(); iter++){
          if((*iter)->isPSM()){ //PSM - actors need to be executed - even if Component can not execute tasks
            Task *newTask = *iter;
            this->taskTracer_.release(newTask);
            assert( readyTasks.find(newTask->getInstanceId())   == readyTasks.end()
                    /* A task can call compute only one time! */);
            assert( runningTasks.find(newTask->getInstanceId()) == runningTasks.end()
                    /* A task can call compute only one time! */);
            readyTasks[newTask->getInstanceId()]=newTask;
            scheduler->addedNewTask(newTask);
            newTasks.erase(iter);
            iter = newTasks.begin();
          }
        }
      }else{
        //look for new tasks (they called compute)
        if(disabledTasks.size()>0){
            std::deque<Task*>::iterator iter = disabledTasks.begin();
            for(;disabledTasks.size() > 0 && iter!=disabledTasks.end(); iter++){
              if((*iter)->getScheduledTask()->getActive()){
                  newTasks.push_back(*iter);
                  disabledTasks.erase(iter);
                  iter = disabledTasks.begin();
              }
            }
        }

        while(newTasks.size()>0){
          Task *newTask;
          newTask=newTasks.front();
          ScheduledTask* actor = newTask->getScheduledTask();
          newTasks.pop_front();
          if(actor!=NULL && !actor->getActive()){
              std::cout<<"actor disabled"<<std::endl;
              disabledTasks.push_back(newTask);
          }else{
            DBG_OUT(this->getName() << " received new Task: "
                    << newTask->getName() << " at: "
                    << sc_time_stamp().to_default_time_units() << std::endl);
            this->taskTracer_.release(newTask);
            //insert new task in read list
            assert( readyTasks.find(newTask->getInstanceId())   == readyTasks.end()
                    /* A task can call compute only one time! */);
            assert( runningTasks.find(newTask->getInstanceId()) == runningTasks.end()
                    /* A task can call compute only one time! */);
            readyTasks[newTask->getInstanceId()]=newTask;
            scheduler->addedNewTask(newTask);
            }
        }
      }
    }

    /**
     *
     */
    void moveToRemainingPipelineStages(Task* task){
      sc_time now                 = sc_time_stamp();
      sc_time restOfLatency       = task->getLatency()  - task->getDelay();
      sc_time end                 = now + restOfLatency;
      if(end <= now){
        //early exit if (Latency-DII) <= 0
        //std::cerr << "Early exit: " << task->getName() << std::endl;
        this->taskTracer_.finishLatency(task);
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

    /**
     *
     */
    virtual void remainingPipelineStages(){
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

            this->taskTracer_.finishLatency(front.task);

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
    void schedule_thread(){
      sc_time timeslice;
      sc_time actualRemainingDelay;
      sc_time *overhead = new sc_time( SC_ZERO_TIME );
      int actualRunningIID;
      bool newTaskDuringOverhead=false;
      //wait(SC_ZERO_TIME);

      scheduler->initialize();
      fireStateChanged(ComponentState::IDLE);
      std::string logName = getName();
      logName = logName + ".buffer";
      std::ofstream logBuffer(logName.c_str());
      if( !logBuffer.is_open() ){
        assert(false);
      }
      unsigned int last_used_buffer = 0;
      //logBuffer << last_used_buffer << " " << sc_time_stamp() << std::endl;

      //QUICKFIX solve thread initialization: actors are released before schedule_thread is called
      newTaskDuringOverhead=(newTasks.size()>0);

      while(1){
      //  std::cout<<"Component " << this->getName() << "schedule_thread @ " << sc_time_stamp() << std::endl;
        //determine the time slice for next scheduling decision and wait for
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
              if(!pendingTask && !hasWaitingOrRunningTasks()){
                if(!requestShutdown()){
                  notify_scheduler_thread.notify();
                }
              }
              wait( notify_scheduler_thread );
            }
          }else{                                        // a task already runs
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
              this->taskTracer_.finishDii(task);
              runningTasks.erase(actualRunningIID);

              task->getBlockEvent().dii->notify();

              if(multiCastGroups.size() != 0 && multiCastGroups.find(task->getProcessId()) != multiCastGroups.end()){
               for(std::list<MultiCastGroupInstance*>::iterator list_iter = multiCastGroupInstances.begin();
                           list_iter != multiCastGroupInstances.end(); list_iter++)
                 {
                   MultiCastGroupInstance* mcgi = *list_iter;
                   if(mcgi->task == task){
                       for(std::list<Task*>::iterator tasks_iter = mcgi->additional_tasks->begin();
                           tasks_iter != mcgi->additional_tasks->end(); tasks_iter++){
                           (*tasks_iter)->getBlockEvent().dii->notify();
                           if((*tasks_iter)->hasScheduledTask()){
                             assert(((*tasks_iter)->getScheduledTask())->canFire());
                             ((*tasks_iter)->getScheduledTask())->schedule();
//                             assert(Director::canExecute((*tasks_iter)->getProcessId()));
//                             Director::execute((*tasks_iter)->getProcessId());
                           }
                           this->taskTracer_.finishDii((*tasks_iter));
                           this->taskTracer_.finishLatency((*tasks_iter));
                           Director::getInstance().signalLatencyEvent((*tasks_iter));
                       }
                       multiCastGroupInstances.remove(mcgi);
                       delete(mcgi->additional_tasks);
                       delete(mcgi);
                       break;
                   }
                 }
              }

              if(task->hasScheduledTask()){
                assert((task->getScheduledTask())->canFire());
                (task->getScheduledTask())->schedule();
//                assert(Director::canExecute(task->getProcessId()));
//                Director::execute(task->getProcessId());
              }
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

        if(!pendingTask && !hasWaitingOrRunningTasks())
          if(!requestShutdown()){
            notify_scheduler_thread.notify();
          }

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
          this->taskTracer_.resign(readyTasks[taskToResign]);
        }

        sc_time timestamp=sc_time_stamp();
        if( overhead != NULL ) delete overhead;
        overhead=scheduler->schedulingOverhead();

        if( overhead != NULL ){
          //    actual time    < endtime
          while( (sc_time_stamp() < timestamp + (*overhead)) ){

            wait( (timestamp+(*overhead))-sc_time_stamp(),
                  notify_scheduler_thread );

          }

          // true if some task becames ready during overhead waiting
          newTaskDuringOverhead=(newTasks.size()>0);
        }else {
          // avoid failures
          overhead=new sc_time(SC_ZERO_TIME);
        }


        //assign task
        if(decision==ONLY_ASSIGN || decision==PREEMPT){
          runningTasks[taskToAssign]=readyTasks[taskToAssign];
          this->taskTracer_.assign(runningTasks[taskToAssign]);
          readyTasks.erase(taskToAssign);
          actualRunningIID=taskToAssign;
          DBG_OUT("IID: " << taskToAssign << "> remaining delay for "
               << runningTasks[taskToAssign]->getName());
          actualRemainingDelay
            = sc_time(runningTasks[taskToAssign]->getRemainingDelay());
          DBG_OUT(" is " << runningTasks[taskToAssign]->getRemainingDelay()
               << endl);

          /* */
          Task * assignedTask = runningTasks[taskToAssign];

          /*
           * Assuming PSM actors are assigned to the same component they model, the executing state of the component should be IDLE
           */
          if (assignedTask->isPSM() == true){
              this->fireStateChanged(ComponentState::IDLE);
          }else{
              this->fireStateChanged(ComponentState::RUNNING);
          }

          if(assignedTask->isBlocking() /* && !assignedTask->isExec() */) {
            blockMutex++;
            if(blockMutex == 1) {
              DBG_OUT(this->getName() << " scheduled blocking task: "
                      << assignedTask->getName() << std::endl);
              assignedTask->ackBlockingCompute();
              DBG_OUT(this->getName() << " enter wait: " << std::endl);
              fireStateChanged(ComponentState::STALLED);
              this->taskTracer_.block(assignedTask);
              while(!assignedTask->isExec()){
                blockCompute.reset();
                CoSupport::SystemC::wait(blockCompute);
                this->addTasks();
              }
              DBG_OUT(this->getName() << " exit wait: " << std::endl);
              fireStateChanged(ComponentState::RUNNING);
              this->taskTracer_.assign(assignedTask);
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
                this->taskTracer_.finishDii(assignedTask);
                //FIXME: notify latency ??
                //assignedTask->traceFinishTaskLatency();
                runningTasks.erase(actualRunningIID);

              }
            }else{
              assert(blockMutex>1);
              scheduler->removedTask(assignedTask);
              fireStateChanged(ComponentState::IDLE);
              this->taskTracer_.finishDii(assignedTask);
              //FIXME: notify latency ??
              //assignedTask->traceFinishTaskLatency();
              runningTasks.erase(actualRunningIID);
              assignedTask->abortBlockingCompute();
            }
            blockMutex--;
          }
          /* */
        }
        if(readyTasks.size() != last_used_buffer){
          //logBuffer<< readyTasks.size() << " " << sc_time_stamp() << std::endl;
          last_used_buffer = readyTasks.size();
          if(last_used_buffer > max_used_buffer){
            max_used_buffer = readyTasks.size();
            //std::cout<<"MAX used Buffer of Component " << this->getName() << " increased to " << max_used_buffer << " @ " << sc_time_stamp() << std::endl;
          }
        }
      }
      //fixme: close is never reached cause of while
      logBuffer.close();
    }


    void compute(Task* actualTask){
      if(multiCastGroups.size() != 0 && multiCastGroups.find(actualTask->getProcessId()) != multiCastGroups.end()){
          //MCG vorhanden und Task auch als MultiCast zu behandeln
          MultiCastGroupInstance* instance = getMultiCastGroupInstance(actualTask);

          if(instance->task != actualTask){
            //instance already running...
            if(instance->task->getBlockEvent().latency->getDropped()){
                //handling of buffer overflow
                actualTask->getBlockEvent().latency->setDropped(true);
            }else{
                ProcessId pid = actualTask->getProcessId();
                ProcessControlBlockPtr pcb = this->getPCB(pid);
                      actualTask->setPCB(pcb);
              this->taskTracer_.release(actualTask);
            }
              return;
          }
      }
      PreemptiveComponent::compute(actualTask);
    }

private:
    TASKTRACER taskTracer_;

  };

} 

#endif /*HSCD_VPC_COMPONENT_H*/
