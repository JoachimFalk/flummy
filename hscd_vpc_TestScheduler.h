#ifndef HSCD_VPC_TESTSCHEDULER_H
#define HSCD_VPC_TESTSCHEDULER_H
#include "systemc.h"
#include "hscd_vpc_Scheduler.h"
#include "hscd_vpc_datatypes.h"
#include <map.h>
#include <deque.h>


class Component;

class TestScheduler : public Scheduler, public sc_module{
 public:
 
    SC_CTOR(TestScheduler){
    SC_THREAD(schedule_thread);
   // cerr << "Scheduler: rr"<<endl;
    //    TIMESLICE=15;
  }
  // map<int,p_struct> getReadyTasks();
  // map<int,p_struct> getRunningTasks();
  sc_event notify_scheduler;


  sc_event& getNotifyEvent();
  void registerComponent(Component *comp);
  action_struct* getNextNewCommand(int pid);

  void schedule_thread();

  virtual ~TestScheduler();
 protected:
  virtual int getSchedulerTimeSlice(sc_time &time)=0;
  virtual void addedNewTask(int pid)=0;
  virtual void removedTask(int pid)=0;
  virtual scheduling_decision schedulingDecision(int& task_to_resign, int& task_to_assign)=0;
  map<int,action_struct> *open_commands;
  map<int,p_struct> ready_tasks,running_tasks;
  deque<int> rr_fifo;
  double TIMESLICE;
  Component *component;
};
#endif
