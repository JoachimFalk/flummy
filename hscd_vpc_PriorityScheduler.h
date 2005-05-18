#ifndef HSCD_VPC_PRIORITYSCHEDULER_H
#define HSCD_VPC_PRIORITYSCHEDULER_H
#include "systemc.h"
#include "hscd_vpc_Scheduler.h"
#include "hscd_vpc_datatypes.h"
#include <map.h>
#include <vector.h>
class Component;

class PriorityScheduler : public Scheduler, public sc_module{
 public:
  SC_CTOR(PriorityScheduler){
    SC_THREAD(schedule_thread);
    //   cerr << "Scheduler: ps"<<endl;
 }
  //  map<int,p_struct> getReadyTasks();
  // map<int,p_struct> getRunningTasks();
  void registerComponent(Component *comp);
  sc_event& PriorityScheduler::getNotifyEvent();
  sc_event notify_scheduler;

 
  void schedule_thread();
  action_struct* getNextNewCommand(int pid);

  virtual ~PriorityScheduler();
 protected:
  map<int,action_struct> *open_commands;
  map<int,p_struct> ready_tasks,running_tasks;
  Component *component;
};
#endif
