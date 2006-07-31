
#ifndef HSCD_VPC_ROUNDROBINSCHEDULER_H
#define HSCD_VPC_ROUNDROBINSCHEDULER_H
#include <systemc.h>
#include <hscd_vpc_Scheduler.h>
#include <hscd_vpc_datatypes.h>
#include <map.h>
#include <deque.h>

namespace SystemC_VPC{
  class Component;

  class RoundRobinScheduler : public Scheduler{
  public:

    RoundRobinScheduler(){
      TIMESLICE=10;
      this->lastassign=0;
      this->remainingSlice=0;
    }
    RoundRobinScheduler(const char *schedulername);
    virtual ~RoundRobinScheduler(){}
    bool getSchedulerTimeSlice(sc_time &time,const map<int,ProcessControlBlock*> &ready_tasks,const map<int,ProcessControlBlock*> &running_tasks);
    void addedNewTask(ProcessControlBlock *pcb);
    void removedTask(ProcessControlBlock *pcb);
    sc_event& getNotifyEvent();
    scheduling_decision schedulingDecision(int& task_to_resign, int& task_to_assign,const  map<int,ProcessControlBlock*> &ready_tasks,const  map<int,ProcessControlBlock*> &running_tasks);
    void setProperty(char* key, char* value);
    sc_time* schedulingOverhead();
    
    void signalDeallocation();
    void signalAllocation();
    
  protected:
    deque<int> rr_fifo;
    double TIMESLICE;
    double lastassign;
    double remainingSlice;
  };
}
#endif
