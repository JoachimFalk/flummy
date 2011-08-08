#ifndef SECONDARYSCHEDULER_H
#define SECONDARYSCHEDULER_H
#include <systemc.h>
#include "Scheduler.hpp"
#include <systemcvpc/datatypes.hpp>
#include <map>
#include <deque>



namespace SystemC_VPC{
  class Component;

  struct AsynchSlot{
      sc_time length;
      int process;
      int Id;
      int priority;
      std::string name;
    };

class MostSecondaryScheduler :public Scheduler{
public:
  MostSecondaryScheduler(){
    sysFreq = 48000;
  }


  sc_time cycle(int sysFreq);

  void addedNewTask(Task *task);

  void removedTask(Task *task);

  bool getSchedulerTimeSlice(sc_time& time,
                              const TaskMap &ready_tasks,
                              const TaskMap &running_tasks);

  scheduling_decision schedulingDecision(
       int& task_to_resign,
       int& task_to_assign,
       const  TaskMap &ready_tasks,
       const  TaskMap &running_tasks);

  sc_time* schedulingOverhead(){}


private:

std::deque<AsynchSlot> Asynch_slots;
sc_time lastassignasynch;
int sysFreq;


};

}

#endif
