#ifndef HSCD_VPC_RATEMONOTONICSCHEDULER_H
#define HSCD_VPC_RATEMONOTONICSCHEDULER_H
#include <systemc.h>
#include <hscd_vpc_Scheduler.h>
#include <hscd_vpc_datatypes.h>
#include <map>
#include <queue>
#include <vector>

namespace SystemC_VPC{
  class Component;

  class RateMonotonicScheduler : public Scheduler{
  public:

    RateMonotonicScheduler(){
      order_counter=0;
    }
    RateMonotonicScheduler(const char *schedulername);
    virtual ~RateMonotonicScheduler(){}
    bool getSchedulerTimeSlice(sc_time &time,const std::map<int,ProcessControlBlock*> &ready_tasks,const std::map<int,ProcessControlBlock*> &running_tasks);
    void addedNewTask(ProcessControlBlock *pcb);
    void removedTask(ProcessControlBlock *pcb);
    sc_event& getNotifyEvent();
    scheduling_decision schedulingDecision(int& task_to_resign, int& task_to_assign,const  std::map<int,ProcessControlBlock*> &ready_tasks,const  std::map<int,ProcessControlBlock*> &running_tasks);
    void setProperty(char* key, char* value);
    sc_time* schedulingOverhead(){return 0;}//;
  protected:
    int order_counter;
    rm_queue_compare comp;
    std::priority_queue<p_queue_entry,std::vector<p_queue_entry>,rm_queue_compare> pqueue;


  };
}
#endif
