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

#ifndef TIMETRIGGEREDCCSCHEDULER_H
#define TIMETRIGGEREDCCSCHEDULER_H
#include <systemcvpc/datatypes.hpp>
#include "Scheduler.hpp"
#include "TDMAScheduler.hpp"
#include "FlexRayScheduler.hpp"

#include <systemc.h>

#include <map>
#include <deque>

namespace SystemC_VPC{
  class Component;
  
  /*Dient zur Speicherung der TDMA-Zeitschlitz - Daten
    pid_fifo enthaelt die laufbereiten Prozesse  
  */

  class TimeTriggeredCCScheduler : public Scheduler{
  public:
    
//   TimeTriggeredCCScheduler(){
//      this->lastassign=sc_time(0,SC_NS);
//      this->remainingSlice=sc_time(0,SC_NS);
//      slicecount=0;
//      curr_slicecount=0;
//    }
    
    TimeTriggeredCCScheduler();
    
    virtual ~TimeTriggeredCCScheduler(){}
    
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
    
    sc_time* schedulingOverhead();
    
    void initialize();
    
  private:
    void _setProperty(const char* key, const char* value);
    
    sc_time lastassign;
    sc_time remainingSlice;
    int slicecount;
    int curr_slicecount;
    int curr_slicecountA;
    int curr_slicecountB;
    int processcount;
    int cyclecount;
    std::vector<TDMASlot> TDMA_slots;
    std::map <ProcessId,int> PIDmap;
    std::map <std::string,struct SlotParameters> ProcessParams_string;
    std::map <ProcessId,struct SlotParameters> ProcessParams;
    std::deque<std::pair<std::string, std::string> > _properties;
    
    //Neu fuer FlexRay
    std::vector<TDMASlot> Dynamic_slots;
    int StartslotDynamic;
    sc_time lastassignA;
    sc_time remainingSliceA;
    sc_time lastassignB;
    sc_time remainingSliceB;
    int taskAssignedToA;
    int taskAssignedToB;
    bool dualchannel;
    sc_time TimeDynamicSegment;   
    
  };
}
#endif
