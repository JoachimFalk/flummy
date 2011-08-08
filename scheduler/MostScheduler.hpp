#ifndef MOSTSCHEDULER_H
#define MOSTSCHEDULER_H
#include <systemc.h>
#include "Scheduler.hpp"
#include <systemcvpc/datatypes.hpp>
#include <map>
#include <deque>
#include <string.h>
#include "MostSecondaryScheduler.hpp"



namespace SystemC_VPC{
  class Component;

  typedef size_t ProcessId;

  struct MostSlot{
    sc_time length;
    int process;
    int Id;
    std::string name;
  };
  
  class MostScheduler : public Scheduler{
  public:
    
    MostScheduler()
      : secondaryScheduler() {

      slicecount = 0;
      streamcount = 0;
      lastassign = SC_ZERO_TIME;
      this->remainingSlice = SC_ZERO_TIME;
      curr_slicecount = -1;
      sysFreq = 48000;
      cycleSize = 372;
      std::map<sc_time, unsigned int> IDmap;

      currSlotStartTime = sc_time(0, SC_NS);
    }
    
    MostScheduler(const char *schedulername);
    
    
    bool getSchedulerTimeSlice(sc_time &time,
                               const TaskMap &ready_tasks,
                               const TaskMap &running_tasks);

    sc_time setboundary(int sysFreq,int framesize);

    sc_time cycle(int sysFreq);

    bool area(int sysFreq,int framesize);
 
    void addedNewTask(Task *task);
    
    void removedTask(Task *task);
    
    sc_event& getNotifyEvent();
    
    scheduling_decision schedulingDecision(int& task_to_resign,
                                           int& task_to_assign,
                                           const  TaskMap &ready_tasks,
                                           const  TaskMap &running_tasks);
    
    sc_time* schedulingOverhead(){
	return NULL;
    }
    
    void initialize(){}

    void addStream(ProcessId pid);
    void closeStream(ProcessId pid);

  private:

    
    std::map<sc_time, unsigned int> slotOffsets;

    std::map<ProcessId, bool> areaMap;
	
    MostSecondaryScheduler secondaryScheduler;

    sc_time lastassign;
    sc_time remainingSlice;
    int slicecount;
    int curr_slicecount;
    int curr_slicecount_help;
    bool already_avail;
    std::deque<MostSlot> Most_slots;
    int sysFreq;
    int cycleSize;
    int streamcount;
    bool flag;
    sc_time currSlotStartTime;
  };
}
#endif
