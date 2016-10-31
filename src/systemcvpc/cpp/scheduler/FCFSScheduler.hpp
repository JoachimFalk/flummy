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

#ifndef HSCD_VPC_FCFSSCHEDULER_H
#define HSCD_VPC_FCFSSCHEDULER_H
#include <systemcvpc/datatypes.hpp>
#include "Scheduler.hpp"

#include <systemc.h>

#include <map>
#include <deque>
namespace SystemC_VPC{

  class Component;

  class FCFSScheduler : public Scheduler{
  public:

    FCFSScheduler(const char *schedulername){
    }
    FCFSScheduler(){
    }
    virtual ~FCFSScheduler(){}
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
    sc_time* schedulingOverhead(){return 0;}//new sc_time(1,SC_NS);
  protected:

    std::deque<int> fcfs_fifo;
    //  double TIMESLICE;
  };
}
#endif
