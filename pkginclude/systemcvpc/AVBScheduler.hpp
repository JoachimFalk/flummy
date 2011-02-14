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

#ifndef AVBSCHEDULER_H
#define AVBSCHEDULER_H
#include <systemc.h>
#include <map>
#include <queue>
#include <vector>

#include "Scheduler.hpp"
#include "datatypes.hpp"
#include "PriorityScheduler.hpp"

namespace SystemC_VPC{
  class Component;
  


  class AVBListEntry{
  public:
    AVBListEntry(){
      bw_alloc = 0;
      priority_level = 0;
      task_queue = 0;
      queue_credit = sc_core::SC_ZERO_TIME;
      has_credit_flag = true;
    }

    AVBListEntry(std::queue<Task*>* t_queue, int p_level, float b_alloc){
      bw_alloc = b_alloc;
      priority_level = p_level;
      task_queue = t_queue;
      queue_credit = sc_core::SC_ZERO_TIME;
      has_credit_flag = true;
    }

    float bw_alloc;
    sc_time queue_credit;
    bool has_credit_flag;
    int priority_level;
    std::queue<Task*>* task_queue;

    int get_priority_level(void){
      return priority_level;
    }

    float get_bw_alloc(void){
      return bw_alloc;
    }

    void increment_credit(sc_time increment){
    if(has_credit()){
  std::cout<<"increment_Credit for class " <<get_priority_level() << " with "<< increment << " @ " << sc_time_stamp()<< std::endl;
	queue_credit += increment;
      }else{
	if(increment < queue_credit){
	  queue_credit - increment;
	}else{
	  queue_credit = increment - queue_credit;
	  has_credit_flag = true;
	}
      }
    }

    void decrement_credit(sc_time decrement){
      std::cout<<"decrement_Credit for class " <<get_priority_level() << " with "<< decrement << " @ " << sc_time_stamp() << std::endl;
      if(decrement < queue_credit){
	queue_credit -= decrement;
      }else{
	queue_credit = decrement - queue_credit;
	has_credit_flag = false;
      }
    }

    bool has_credit(void){
      //return (queue_credit >= sc_core::SC_ZERO_TIME);
      return has_credit_flag;
      
    }

    void reset_credit(void){
      //return (queue_credit >= sc_core::SC_ZERO_TIME);
std::cout<<"reset_credit for class " <<get_priority_level() << " to "<< sc_core::SC_ZERO_TIME << " @ " << sc_time_stamp() << std::endl;
      has_credit_flag = true;
      queue_credit = sc_core::SC_ZERO_TIME;
      
    }

    sc_time get_credit(void){
      if(has_credit_flag){
	return queue_credit;
      }else{
	return sc_time(-1, SC_NS);
      }
    }

  };

  class AVBScheduler : public Scheduler{
  public:

    AVBScheduler();
    virtual ~AVBScheduler(){}
    bool getSchedulerTimeSlice(sc_time &time,
                               const TaskMap &ready_tasks,
                               const TaskMap &running_tasks);
    void addedNewTask(Task *task);
    void removedTask(Task *task);
    sc_event& getNotifyEvent();
    scheduling_decision schedulingDecision(int& task_to_resign,
                                           int& task_to_assign,
                                           const  TaskMap &ready_tasks,
                                           const  TaskMap &running_tasks);
    void setProperty(const char* key, const char* value);
    void setAttribute(AttributePtr attributePtr);

    sc_time* schedulingOverhead(){return 0;}//;
  protected:
    std::list<AVBListEntry*> p_list;
    std::deque<std::pair<std::string, std::string> > _properties;
    int last_active;
    sc_time time_last_assign;

  };
}
#endif
