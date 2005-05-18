
#ifndef HSCD_VPC_SIMPLESCHEDULER_H
#define HSCD_VPC_SIMPLESCHEDULER_H
#include "systemc.h"
#include "hscd_vpc_Scheduler.h"
#include "hscd_vpc_datatypes.h"
#include <map.h>
#include <deque.h>


class Component;

class SimpleScheduler : public Scheduler, public sc_module{
 public:
  SC_CTOR(SimpleScheduler){
    SC_THREAD(schedule_thread);
    // cerr << "Scheduler: rr"<<endl;
    TIMESLICE=15;
  }
  //  map<int,p_struct> getReadyTasks();
  // map<int,p_struct> getRunningTasks();
  void registerComponent(Component *comp);
  sc_event& SimpleScheduler::getNotifyEvent();
  sc_event notify_scheduler;


  void schedule_thread();
  action_struct* getNextNewCommand(int pid);

  virtual ~SimpleScheduler();
 protected:
  int getSchedulerTimeSlice(sc_time& time);
  map<int,action_struct> *open_commands;
  map<int,p_struct> ready_tasks,running_tasks;
  deque<int> rr_fifo;
  double TIMESLICE;
  Component *component;
};
#endif
