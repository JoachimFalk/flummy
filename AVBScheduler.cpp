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

#include <systemcvpc/AVBScheduler.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/Component.hpp>
#include <systemcvpc/datatypes.hpp>

namespace SystemC_VPC{
  
  AVBScheduler::AVBScheduler(const char *schedulername){
    AVBListEntry* newEntry;
    //default-IP-traffic (Priority 0) - is allowed to use LineSpeed
    newEntry = new AVBListEntry( new std::queue<Task*>, 0, 1.0);
    p_list.insert(p_list.begin(), newEntry);
    last_active = -1;
    time_last_assign = sc_time_stamp();
  
  }

  void AVBScheduler::setProperty(const char* key, const char* value){
    std::pair<std::string, std::string> toadd(key,value);
    _properties.push_back(toadd);
    std::cout<<"unused function" << std::endl;
    assert(0);
  }


  void AVBScheduler::setAttribute(AttributePtr attributePtr){
    std::cout<<"AVBScheduler::setAttribute " << attributePtr->getType() << " " << attributePtr->getValue()  << std::endl;
  
    std::istringstream is(attributePtr->getType());
    int priority;
    is >> priority;

    AVBListEntry* newEntry;
    std::istringstream is2(attributePtr->getValue());
    float value;
    is2 >> value;
    
    newEntry = new AVBListEntry( new std::queue<Task*>, priority, value);

    std::cout<<"created new AVBListEntry with priority: "<< priority << " and value: " << value << std::endl;

    std::list<AVBListEntry*>::iterator it;
    for (it=p_list.begin(); it!=p_list.end(); it++){
      if((*it)->get_priority_level() < priority){
	std::cout<<"added newEntry to p_list" << std::endl;
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
      //find the next time, when a task is allowed to run (it has enought credits)
      sc_time needed_time = sc_core::SC_ZERO_TIME;
      std::list<AVBListEntry*>::iterator it;
      for (it=p_list.begin(); it!=p_list.end(); it++){
	if((*it)->task_queue->size() != 0){
	  assert((*it)->has_credit() == false);
	  sc_time curr_needed_time = (*it)->get_credit() * ((*it)->get_bw_alloc());
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
    std::cout<<"AVBScheduler::addedNewTask " << std::endl;
    int priority = task->getPriority();
    std::list<AVBListEntry*>::iterator it;
    for (it=p_list.begin(); it!=p_list.end(); it++){
	if((*it)->get_priority_level() == priority){
	  (*it)->task_queue->push(task);
	  std::cout<<"Added Task with priority:"<< priority << " to queue " << (*it)->get_priority_level() << " and reservation: " << (*it)->get_bw_alloc() << std::endl;

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
      std::cout<<"AVBScheduler::removedTask " << task << std::endl;
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
  std::cout<<"AVBScheduler::schedulingDecision" << std::endl;
      scheduling_decision ret_decision=NOCHANGE;
    
    std::list<AVBListEntry*>::iterator it;

  if(running_tasks.size()!=0){
    //no preemption! 
std::cout<<"AVBScheduler::schedulingDecision - no Preemption!" << std::endl;

  }else{
    //Update of the Credits
std::cout<<"AVBScheduler::schedulingDecision - Updateing credits?" << std::endl;
    if(last_active == -1 || ready_tasks.size() == 0){ //no active Packet -> reset all credits
      for (it=p_list.begin(); it!=p_list.end(); it++){
	  (*it)->reset_credit();
      }
    }else{
    sc_time time_budget = sc_time_stamp() - time_last_assign;
      for (it=p_list.begin(); it!=p_list.end(); it++){
	if((*it)->task_queue->size()!= 0){
	  if((*it)->get_priority_level() != last_active){
	    //task was blocked by another task (in an other priority_level)
	    //so raise up the credits
	    (*it)->increment_credit(time_budget * (*it)->get_bw_alloc());
	  }else{
	    //decrement the credit of the currently used queue
	    (*it)->decrement_credit(time_budget * (1-(*it)->get_bw_alloc()));
	  }
	}else{
	  (*it)->reset_credit();
	}
      }
    }



  //search the new task
      std::cout<<"AVBScheduler::schedulingDecision - search for new task!" << std::endl;
      for (it=p_list.begin(); it!=p_list.end(); it++){
	  if((*it)->task_queue->size() != 0 && (*it)->has_credit() ){
	      task_to_assign = (*it)->task_queue->front()->getInstanceId();
	      (*it)->task_queue->pop();
	      ret_decision = ONLY_ASSIGN;
	      last_active = (*it)->get_priority_level();
	      time_last_assign = sc_time_stamp();
	      std::cout<<"Found an Entry with priority:"<< (*it)->get_priority_level() << " and credit: " << (*it)->get_credit() << std::endl;
	      return ret_decision;
	  }else{
	    //nothing to do
	  }
      }
std::cout<<"AVBScheduler::schedulingDecision - getting IDLE" << std::endl;
    //no new task was found -> getting idle
    last_active = -1;
    time_last_assign = sc_time_stamp();    
    }
//     
     /*
      if(pqueue.size()<=0) return NOCHANGE;    // kein neuer -> nichts tun

     // hoechste prioritaet�der ready tasks
     p_queue_entry prior_ready=pqueue.top();
     task_to_assign=prior_ready.task->getInstanceId();

     if(running_tasks.size()!=0){
        //nothing to do, NO PREEMPTIVE-Scheduler
     }else{
       pqueue.pop();
       ret_decision=ONLY_ASSIGN;  
     }
  */

     return ret_decision;
   }
}
