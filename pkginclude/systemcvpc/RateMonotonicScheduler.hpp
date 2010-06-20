#ifndef HSCD_VPC_RATEMONOTONICSCHEDULER_H
#define HSCD_VPC_RATEMONOTONICSCHEDULER_H
#include <systemc.h>
#include "Scheduler.hpp"
#include "datatypes.hpp"
#include <map>
#include <queue>
#include <vector>

namespace SystemC_VPC{
  class Component;

  struct rm_queue_compare{
    bool operator()(const p_queue_entry& pqe1,
        const p_queue_entry& pqe2) const
    {
      double p1 = sc_time(1,SC_NS)/pqe1.task->getPeriod();
      double p2 = sc_time(1,SC_NS)/pqe2.task->getPeriod();
      if (p1 > p2)
        return true;
      else if(p1 == p2)
        return (pqe1.fifo_order>pqe2.fifo_order);
      else
        return false;
    }

  };

  class RateMonotonicScheduler : public Scheduler{
  public:

    RateMonotonicScheduler(){
      order_counter=0;
    }
    RateMonotonicScheduler(const char *schedulername);
    virtual ~RateMonotonicScheduler(){}
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
    rm_queue_compare comp;
    std::priority_queue<p_queue_entry,std::vector<p_queue_entry>,rm_queue_compare> pqueue;


  };
}
#endif
