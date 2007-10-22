#ifndef FLEXRAYSCHEDULER_H
#define FLEXRAYSCHEDULER_H
#include <systemc.h>
#include <hscd_vpc_Scheduler.h>
#include <hscd_vpc_datatypes.h>
#include <map>
#include <deque>
#include <TDMAScheduler.h>

namespace SystemC_VPC{
  class Component;
  
  /*Dient zur Speicherung der TDMA-Zeitschlitz - Daten
  	pid_fifo enthält die laufbereiten Prozesse  
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
    
    //Neu für FlexRay
    std::vector<TDMASlot> Dynamic_slots; //<-- brauch ich nicht, wenn ich alles in TDMA_slots packe
    int StartslotDynamic;
    sc_time lastassignA;
    sc_time remainingSliceA;
    sc_time lastassignB;
    sc_time remainingSliceB;
    int taskAssignedToA;
    int taskAssignedToB;
    sc_time TimeDynamicSegment;
    //FlexRay-Parameter
    int vCycleCount; 			//zählt die bereits abgelaufenen CommunicationCycles
    int vSlotCounter[2]; 		//Enthält den aktuellen Slot; [0] = KanalA , [1] = KanalB
    static int cSlotIDMax;		//Max. SlotID bevor Cycle wiederholt wird
    static int cCycleCountMax; 	 	//Max. Anzahl an Cycles von vCycleCount vor Rücksetzung auf 1
    static int gdStaticSlot;  		//Anzahl Macroticks pro staticSlot
    static int gNumberOfStaticSlots;	//Anzahl statischer (TDMA) Slots
    static int gdMinislot;  		//Anzahl Macroticks pro Minislot
    static int gNumberOfMinislots; 	//Anzahl der Minislots im dyn. Segment
    static int gdSymbolWindow;  	//Anzahl Macroticks des Symbol-Windows ( >=0)
    static int gdActionPointOffset;	//Enthält Versatz zwischen stat. Slotbeginn und Startzeitpunkt
    static int gdMinislotActionPointOffset;  //Enthält Versatz zwischen dyn. Slotgeinn und Startzeitpunkt
        
    
  };
}
#endif

