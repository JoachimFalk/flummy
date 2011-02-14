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

#include <systemcvpc/RoundRobinScheduler.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/ComponentImpl.hpp>

namespace SystemC_VPC{

  void RoundRobinScheduler::setProperty(const char* key, const char* value){
    if(0==strncmp(key,"timeslice",strlen("timeslice"))){
      const char *domain;
      domain=strstr(value,"ns");
      if(domain!=NULL){
        //domain[0]='\0';
        sscanf(value,"%lf",&TIMESLICE);
      }

    }
  }

  bool RoundRobinScheduler::getSchedulerTimeSlice(
    sc_time& time,
    const TaskMap &ready_tasks,
    const  TaskMap &running_tasks )
  {
    if(rr_fifo.size()==0 && running_tasks.size()==0) return 0;
    time=sc_time(TIMESLICE,SC_NS);
    return true;
  }
  void RoundRobinScheduler::addedNewTask(Task *task){
    rr_fifo.push_back(task->getInstanceId());
  }
  void RoundRobinScheduler::removedTask(Task *task){
    std::deque<int>::iterator iter;
    for(iter=rr_fifo.begin();iter!=rr_fifo.end();iter++){
      if( *iter == task->getInstanceId()){
        rr_fifo.erase(iter);
        break;
      }
    }
  }
  scheduling_decision RoundRobinScheduler::schedulingDecision(
    int& task_to_resign,
    int& task_to_assign,
    const  TaskMap &ready_tasks,
    const  TaskMap &running_tasks )
  {

    scheduling_decision ret_decision=NOCHANGE;

    this->remainingSlice = this->remainingSlice -
      (sc_time_stamp().to_default_time_units() - this->lastassign);
    this->lastassign = sc_time_stamp().to_default_time_units();

    if(this->remainingSlice <= 0){// time slice expired
      if(rr_fifo.size()>0){    // select next task
        task_to_assign = rr_fifo.front();
        rr_fifo.pop_front();
        
        // default: old tasks execution delay is expired (no running task)
        ret_decision= ONLY_ASSIGN;
        if(running_tasks.size()!=0){  // a running task is preempted
          TaskMap::const_iterator iter;
          iter=running_tasks.begin();
          Task *task=iter->second;
          task_to_resign=task->getInstanceId();
          rr_fifo.push_back(task->getInstanceId());
          ret_decision= PREEMPT;  
        }
      }    
    }else{
      // either a new task was added
      // or the running tasks delay is expired

      if(running_tasks.size()==0){       // if running tasks delay has expired
        if(rr_fifo.size()>0){            // schedule a new task
          task_to_assign = rr_fifo.front();
          rr_fifo.pop_front();

          // this is not preemption: the old task BLOCKED
          // and a new one is assigned
          ret_decision= ONLY_ASSIGN;
        }
      }
    } 
    return ret_decision;
  }


  /**
   *
   */
  sc_time* RoundRobinScheduler::schedulingOverhead(){
    return NULL; //new sc_time(1,SC_NS);
  }
}
