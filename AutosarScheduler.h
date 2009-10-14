#ifndef AUTOSARSCHEDULER_H
#define AUTOSARSCHEDULER_H
#include <systemc.h>
#include "hscd_vpc_Scheduler.h"
#include "hscd_vpc_datatypes.h"
#include <map>
#include <deque>
#include "TDMAScheduler.h"
#include "hscd_vpc_PrioritySchedulerNoPreempt.h"
#include "FlexRayScheduler.h"

namespace SystemC_VPC{
  class Component;
  
  /*
Properties of AutosarScheduler:
  - Time-triggered static slots with cycle-multiplexing 
  - dynamic part is scheduled by non-preemptive Priority-based Scheduler
*/
  class AutosarScheduler : public Scheduler{
  public:
    
    AutosarScheduler(): scheduler_dynamic("Autosar"){
      this->lastassign=sc_time(0,SC_NS);
      this->remainingSlice=sc_time(0,SC_NS);
      slicecount=0;
      curr_slicecount=0;
    }
    
    AutosarScheduler(const char *schedulername);
    
    virtual ~AutosarScheduler(){}
    
    bool getSchedulerTimeSlice(sc_time &time,const TaskMap &ready_tasks,const TaskMap &running_tasks);
    
    void addedNewTask(Task *task);
    
    void removedTask(Task *task);
    
    sc_event& getNotifyEvent();
    
    scheduling_decision schedulingDecision(int& task_to_resign, int& task_to_assign,const  TaskMap &ready_tasks,const  TaskMap &running_tasks);
    
    void setProperty(const char* key, const char* value);
    
    void setAttribute(Attribute& fr_Attribute);
    
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
    sc_time TimeDynamicSegment;
    PrioritySchedulerNoPreempt scheduler_dynamic;
    
  };
}
#endif

