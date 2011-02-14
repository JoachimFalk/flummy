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

#ifndef FLEXRAYSCHEDULER_H
#define FLEXRAYSCHEDULER_H
#include <systemc.h>
#include "Scheduler.hpp"
#include "datatypes.hpp"
#include <map>
#include <deque>
#include "TDMAScheduler.hpp"

namespace SystemC_VPC{
  class Component;
  
  /*Dient zur Speicherung der TDMA-Zeitschlitz - Daten
    pid_fifo enthaelt die laufbereiten Prozesse  
  */
  struct SlotParameters{
	SlotParameters(int offset, int multiplex)
		: offset(offset), multiplex(multiplex){}
	SlotParameters()
		: offset(0), multiplex(0){}
    int offset; //in cycles
    int multiplex; //in 2^multiplex - cycles
  };
  
  class FlexRayScheduler : public Scheduler{
  public:
    
//    FlexRayScheduler(){
//      this->lastassign=sc_time(0,SC_NS);
//      this->remainingSlice=sc_time(0,SC_NS);
//      slicecount=0;
//      curr_slicecount=0;
//    }
    
    FlexRayScheduler();
    
    virtual ~FlexRayScheduler(){}
    
    bool getSchedulerTimeSlice(sc_time &time,const TaskMap &ready_tasks,const TaskMap &running_tasks);
    
    void addedNewTask(Task *task);
    
    void removedTask(Task *task);
    
    sc_event& getNotifyEvent();
    
    scheduling_decision schedulingDecision(int& task_to_resign, int& task_to_assign,const  TaskMap &ready_tasks,const  TaskMap &running_tasks);
    
    void setProperty(const char* key, const char* value);
    
    void setAttribute(AttributePtr attributePtr);
    
    sc_time* schedulingOverhead();
    
    void initialize();
    
  private:
    void _setProperty(const char* key, const char* value);
    
    sc_time lastassign;
    sc_time cycle_length;
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
    bool to_init;
   // bool firstrun;
    sc_time TimeDynamicSegment;   
    
  };
}
#endif

