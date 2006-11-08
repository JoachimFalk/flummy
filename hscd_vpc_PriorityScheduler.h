#ifndef HSCD_VPC_PRIORITYSCHEDULER_H
#define HSCD_VPC_PRIORITYSCHEDULER_H
#include <systemc.h>
#include <hscd_vpc_Scheduler.h>
#include <hscd_vpc_datatypes.h>
#include <map>
#include <queue>
#include <vector>
using namespace std;

namespace SystemC_VPC{
  class Component;

  class PriorityScheduler : public Scheduler{
  public:

    PriorityScheduler(){
      order_counter=0;
    }
    PriorityScheduler(const char *schedulername);
    virtual ~PriorityScheduler(){}
    bool getSchedulerTimeSlice(sc_time &time,const map<int,ProcessControlBlock*> &ready_tasks,const map<int,ProcessControlBlock*> &running_tasks);
    void addedNewTask(ProcessControlBlock *pcb);
    void removedTask(ProcessControlBlock *pcb);
    sc_event& getNotifyEvent();
    scheduling_decision schedulingDecision(int& task_to_resign, int& task_to_assign,const  map<int,ProcessControlBlock*> &ready_tasks,const  map<int,ProcessControlBlock*> &running_tasks);
    void setProperty(char* key, char* value);
    sc_time* schedulingOverhead(){return 0;}//;
  protected:
    int order_counter;
    p_queue_compare comp;
    priority_queue<p_queue_entry,vector<p_queue_entry>,p_queue_compare> pqueue;

  };
}
#endif
