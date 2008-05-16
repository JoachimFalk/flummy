#ifndef FLEXRAYSCHEDULER_H
#define FLEXRAYSCHEDULER_H
#include <systemc.h>
#include "hscd_vpc_Scheduler.h"
#include "hscd_vpc_datatypes.h"
#include <map>
#include <deque>
#include "TDMAScheduler.h"

namespace SystemC_VPC{
  class Component;
  
  /*Dient zur Speicherung der TDMA-Zeitschlitz - Daten
  	pid_fifo enthaelt die laufbereiten Prozesse  
  */
  
  class FlexRayScheduler : public Scheduler{
  public:
    
   FlexRayScheduler(){
      this->lastassign=sc_time(0,SC_NS);
      this->remainingSlice=sc_time(0,SC_NS);
      slicecount=0;
      curr_slicecount=0;
    }
    
    FlexRayScheduler(const char *schedulername);
    
    virtual ~FlexRayScheduler(){}
    
    bool getSchedulerTimeSlice(sc_time &time,const std::map<int,ProcessControlBlock*> &ready_tasks,const std::map<int,ProcessControlBlock*> &running_tasks);
    
    void addedNewTask(ProcessControlBlock *pcb);
    
    void removedTask(ProcessControlBlock *pcb);
    
    sc_event& getNotifyEvent();
    
    scheduling_decision schedulingDecision(int& task_to_resign, int& task_to_assign,const  std::map<int,ProcessControlBlock*> &ready_tasks,const  std::map<int,ProcessControlBlock*> &running_tasks);
    
    void setProperty(const char* key, const char* value);
    
    void setAttribute(Attribute& fr_Attribute);
    
    sc_time* schedulingOverhead();
    
    void signalDeallocation(bool kill);
    void signalAllocation();
    
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
    std::vector<TDMASlot> TDMA_slots;
    std::map <ProcessId,int> PIDmap;
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

