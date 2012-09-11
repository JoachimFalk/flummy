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

#include <systemcvpc/Director.hpp>
#include <systemcvpc/datatypes.hpp>

#include "AVBScheduler.hpp"
#include "ComponentImpl.hpp"

namespace SystemC_VPC{
  
  AVBScheduler::AVBScheduler(){
    AVBListEntry* newEntry;
    //default-IP-traffic (Priority 0) - is allowed to use LineSpeed
    newEntry = new AVBListEntry( new std::queue<Task*>, INT_MAX, 1.0);
    p_list.insert(p_list.begin(), newEntry);
    last_active = -1;
    firstrun=true;
    time_last_assign = sc_time_stamp();
  
  }

  void AVBScheduler::setProperty(const char* key, const char* value){
    std::pair<std::string, std::string> toadd(key,value);
    _properties.push_back(toadd);
    std::cout<<"unused function" << std::endl;
    assert(0);
  }


  void AVBScheduler::setAttribute(AttributePtr attributePtr){
    std::istringstream is(attributePtr->getType());
    int priority;
    is >> priority;

    AVBListEntry* newEntry;
    std::istringstream is2(attributePtr->getValue());
    float value;
    is2 >> value;
    
    newEntry = new AVBListEntry( new std::queue<Task*>, priority, value);

    std::list<AVBListEntry*>::iterator it;
    for (it=p_list.begin(); it!=p_list.end(); it++){
      if((*it)->get_priority_level() > priority){
	p_list.insert(it, newEntry);
	it = p_list.end();
	it--;
      }
    }
    
  }


  bool AVBScheduler::getSchedulerTimeSlice(
    sc_time& time,
    const TaskMap &ready_tasks,
    const  TaskMap &running_tasks )
  {
    if(ready_tasks.size() == 0 || running_tasks.size() != 0){
      //no task.. or a task is running -> no rescheduling
      return false;
    }else{
      //find the next time, when a task is allowed to run (it has enough credits)
      sc_time needed_time = sc_core::SC_ZERO_TIME;
      std::list<AVBListEntry*>::iterator it;
      for (it=p_list.begin(); it!=p_list.end(); it++){
	if((*it)->task_queue->size() != 0){
	  assert((*it)->has_credit() == false);
	  sc_time curr_needed_time = (SC_ZERO_TIME -(*it)->get_credit()) / ((*it)->get_bw_alloc());
	  if(needed_time == sc_core::SC_ZERO_TIME || needed_time > curr_needed_time){
	    needed_time = curr_needed_time;
	  }
	}
      }
    time = needed_time;
    return true;
    }
  }
  /**
   *
   */
  void AVBScheduler::addedNewTask(Task *task){
    int priority = task->getPriority();
    std::list<AVBListEntry*>::iterator it;
    bool added = false;
    for (it=p_list.begin(); it!=p_list.end(); it++){
	if((*it)->get_priority_level() >= priority && !added){
	  (*it)->task_queue->push(task);
	  if((*it)->task_queue->size()==1){
	    (*it)->setWasEmpty(true);
	  }
	  added = true;
	}
   }

    /*
    p_queue_entry pqe;
    pqe.fifo_order=order_counter++;
    pqe.task=task;
    pqueue.push(pqe);
  */
  }

  /**
   *
   */
  void AVBScheduler::removedTask(Task *task){
  }

  /**
   *
   */
   scheduling_decision AVBScheduler::schedulingDecision(
     int& task_to_resign,
     int& task_to_assign,
     const  TaskMap &ready_tasks,
     const  TaskMap &running_tasks)
   {
    scheduling_decision ret_decision=NOCHANGE;
    std::list<AVBListEntry*>::iterator it;
    if(firstrun){
      //QuickFix for initial credit-error
      time_last_assign=sc_time_stamp() ;
      firstrun=false;
    }
    if(running_tasks.size()!=0){
      //no preemption! Only update the credits!
      //or after an idle phase
      sc_time time_budget = sc_time_stamp() - time_last_assign;
      for (it=p_list.begin(); it!=p_list.end(); it++){
        if((*it)->get_priority_level() != last_active){
          if((*it)->task_queue->size()!= 0){
            (*it)->increment_credit(time_budget * (*it)->get_bw_alloc());
            if((*it)->wasEmpty()){
                if((*it)->has_credit()){
                    (*it)->reset_credit();
                }
            (*it)->setWasEmpty(false);
            }
          }else if(!(*it)->has_credit()){
            (*it)->increment_credit(time_budget * (*it)->get_bw_alloc());
            if((*it)->has_credit()){
              (*it)->reset_credit();
            }
          }
        }else{
          (*it)->setWasEmpty(false);
          (*it)->decrement_credit(time_budget * (1-(*it)->get_bw_alloc()));
        }
      }
      time_last_assign = sc_time_stamp();
    }else{ //Update of the Credits
      sc_time time_budget = sc_time_stamp() - time_last_assign;
      for (it=p_list.begin(); it!=p_list.end(); it++){
	if((*it)->task_queue->size()!= 0 ){
	  if((*it)->get_priority_level() != last_active){
	    //task was blocked by another task (in another priority_level)
	    //OR it was newly added!
	    //so raise up the credits
	    (*it)->increment_credit(time_budget * (*it)->get_bw_alloc());

	    if((*it)->has_credit() && (*it)->wasEmpty()){
	      (*it)->setWasEmpty(false);
	      (*it)->reset_credit();
	    }
	  }else{
	    //decrement the credit of the currently used queue
	    (*it)->decrement_credit(time_budget * (1-(*it)->get_bw_alloc()));
	  }
	}else{
	      if((*it)->get_priority_level() != last_active){
	        (*it)->increment_credit(time_budget * (*it)->get_bw_alloc());
	      }else{
	        (*it)->decrement_credit(time_budget * (1-(*it)->get_bw_alloc()));
	      }
	  if((*it)->has_credit()){
	    (*it)->reset_credit();
	  }
	}
      }

  //search the new task
      for (it=p_list.begin(); it!=p_list.end(); it++){
	  if((*it)->task_queue->size() != 0 && (*it)->has_credit() ){
	      task_to_assign = (*it)->task_queue->front()->getInstanceId();
	      (*it)->task_queue->pop();
	      ret_decision = ONLY_ASSIGN;
	      last_active = (*it)->get_priority_level();
	      time_last_assign = sc_time_stamp();
	      return ret_decision;
	  }else{
	    //nothing to do
	  }
      }
    //no new task was found -> getting idle
    last_active = -1;
    time_last_assign = sc_time_stamp();    
    }

     return ret_decision;
   }
}
