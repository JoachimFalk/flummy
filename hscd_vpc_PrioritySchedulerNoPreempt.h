#ifndef HSCD_VPC_PRIORITYSCHEDULERNOPREEMPT_H
#define HSCD_VPC_PRIORITYSCHEDULERNOPREEMPT_H
#include <systemc.h>
#include <map>
#include <queue>
#include <vector>

#include "hscd_vpc_Scheduler.h"
#include "hscd_vpc_datatypes.h"
#include "hscd_vpc_PriorityScheduler.h"

namespace SystemC_VPC{
  class Component;

  class PrioritySchedulerNoPreempt : public Scheduler{
  public:

    PrioritySchedulerNoPreempt(){
      order_counter=0;
      current_task=NULL;
    }
    PrioritySchedulerNoPreempt(const char *schedulername);
    virtual ~PrioritySchedulerNoPreempt(){}
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
    sc_time* schedulingOverhead(){return 0;}//;
  protected:
    int order_counter;
    p_queue_compare comp;
    std::priority_queue<p_queue_entry,std::vector<p_queue_entry>,p_queue_compare> pqueue;
    
    private:
    Task *current_task;

  };
}
#endif
