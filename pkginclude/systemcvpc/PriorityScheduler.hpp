#ifndef HSCD_VPC_PRIORITYSCHEDULER_H
#define HSCD_VPC_PRIORITYSCHEDULER_H
#include <systemc.h>
#include "Scheduler.hpp"
#include "datatypes.hpp"
#include <map>
#include <queue>
#include <vector>

namespace SystemC_VPC{
  class Component;

  struct p_queue_compare{
    bool operator()(const p_queue_entry& pqe1,
        const p_queue_entry& pqe2) const
    {
      int p1=pqe1.task->getPriority();
      int p2=pqe2.task->getPriority();
      if (p1 > p2)
        return true;
      else if(p1 == p2)
        return (pqe1.fifo_order>pqe2.fifo_order);
      else
        return false;
    }

  };


  class PriorityScheduler : public Scheduler{
  public:

    PriorityScheduler(){
      order_counter=0;
    }
    PriorityScheduler(const char *schedulername);
    virtual ~PriorityScheduler(){}
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

  };
}
#endif
