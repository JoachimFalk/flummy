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

#include "ComponentImpl.hpp"

namespace SystemC_VPC{

  void RoundRobinScheduler::setProperty(const char* key, const char* value){
    if (std::string("timeslice") == key) {
      timeSlice_ = Director::createSC_Time(value);
    }
  }

  bool RoundRobinScheduler::getSchedulerTimeSlice(
    sc_time& time,
    const TaskMap &ready_tasks,
    const  TaskMap &running_tasks )
  {
    if(rr_fifo.size()==0 && running_tasks.size()==0) return 0;
    time=timeSlice_;
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

//
int RoundRobinScheduler::assignFromFront()
{
  int task_to_assign = rr_fifo.front();
  rr_fifo.pop_front();
  timeSliceExpires_ = sc_time_stamp() + timeSlice_;
  return task_to_assign;
}

  scheduling_decision RoundRobinScheduler::schedulingDecision(
    int& task_to_resign,
    int& task_to_assign,
    const  TaskMap &ready_tasks,
    const  TaskMap &running_tasks )
  {

    scheduling_decision ret_decision=NOCHANGE;

    if(sc_time_stamp() == timeSliceExpires_){// time slice expired
      if(rr_fifo.size()>0){    // select next task
        task_to_assign = assignFromFront();
        // default: old tasks execution delay is expired (no running task)
        ret_decision= ONLY_ASSIGN;
        if(!running_tasks.empty()){  // a running task is preempted
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
          task_to_assign = assignFromFront();
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
