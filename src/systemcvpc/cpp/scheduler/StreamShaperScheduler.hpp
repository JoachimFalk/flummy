/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 * 
 * Project title: SystemC-VPC
 * Comment:
 * Author: Graf
 * ----------------------------------------------------------------------------
 */

#ifndef STREAMSHAPERSCHEDULER_H
#define STREAMSHAPERSCHEDULER_H
#include <systemcvpc/datatypes.hpp>
#include "Scheduler.hpp"

#include <systemc.h>

#include <map>
#include <deque>

namespace SystemC_VPC{
  class Component;

  typedef size_t ProcessId;
  typedef std::deque< std::pair <std::string, std::string> > Properties;

  class StreamShaperScheduler : public Scheduler{
  public:
    
    StreamShaperScheduler();
    
    virtual ~StreamShaperScheduler(){}
    
    bool getSchedulerTimeSlice(sc_time &time,
                               const TaskMap &ready_tasks,
                               const TaskMap &running_tasks);
    
    void addedNewTask(Task *task);
    
    void removedTask(Task *task);
    
    void setAttribute(AttributePtr attributePtr);

    sc_event& getNotifyEvent();
    
    scheduling_decision schedulingDecision(int& task_to_resign,
                                           int& task_to_assign,
                                           const  TaskMap &ready_tasks
                                           ,const  TaskMap &running_tasks);
    
    void setProperty(const char* key, const char* value);
    
    sc_time* schedulingOverhead();
    
    void initialize();
    
  private:
    void _setProperty(const char* key, const char* value);
    
    sc_time shapeCycle;
    bool firstrun;
    sc_time lastassign;
    sc_time remainingSlice;
    std::deque<std::pair<std::string, std::string> > _properties;
    std::deque<int> stream_fifo;
  };
}
#endif
