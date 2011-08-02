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

#ifndef HSCD_VPC_ROUNDROBINSCHEDULER_H
#define HSCD_VPC_ROUNDROBINSCHEDULER_H
#include <systemc.h>
#include "Scheduler.hpp"
#include "datatypes.hpp"
#include <map>
#include <deque>

namespace SystemC_VPC{
  class Component;

  class RoundRobinScheduler : public Scheduler{
  public:

    RoundRobinScheduler() :
      timeSlice_(10, SC_NS), timeSliceExpires_()
    {
    }
    virtual ~RoundRobinScheduler(){}
    bool getSchedulerTimeSlice(sc_time &time,
                               const TaskMap &ready_tasks,
                               const TaskMap &running_tasks);
    void addedNewTask(Task *task);
    void removedTask(Task *task);

    //
    scheduling_decision
    schedulingDecision(int& task_to_resign,
                       int& task_to_assign,
                       const  TaskMap &ready_tasks,
                       const  TaskMap &running_tasks);
    void setProperty(const char* key, const char* value);
    sc_time* schedulingOverhead();
    
  private:
    std::deque<int> rr_fifo;
    sc_time timeSlice_;
    sc_time timeSliceExpires_;

    int assignFromFront();
  };
}
#endif
