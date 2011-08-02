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

#ifndef HSCD_VPC_PRIORITYSCHEDULER_H
#define HSCD_VPC_PRIORITYSCHEDULER_H

#include <systemcvpc/datatypes.hpp>
#include "Scheduler.hpp"

#include <systemc.h>

#include <map>
#include <queue>
#include <vector>

namespace SystemC_VPC{
  class Component;

  class PriorityScheduler : public Scheduler{
  public:

    PriorityScheduler() : order_counter(0) {}
    virtual ~PriorityScheduler(){}
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
    sc_time* schedulingOverhead(){return 0;}//;
  protected:
    int order_counter;
    std::priority_queue<p_queue_entry> pqueue;

  };
}
#endif
