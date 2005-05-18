#ifndef HSCD_VPC_SPECIALTESTSCHEDULER_H
#define HSCD_VPC_SPECIALTESTSCHEDULER_H
#include "systemc.h"
#include "hscd_vpc_TestScheduler.h"
#include "hscd_vpc_datatypes.h"
#include <map.h>
#include <deque.h>


class Component;

class SpecialTestScheduler : public TestScheduler{
 public:

  SpecialTestScheduler(sc_module_name name_) : TestScheduler(name_){
    TIMESLICE=15;
  }
  virtual ~SpecialTestScheduler(){}
 protected:
  int getSchedulerTimeSlice(sc_time &time);
  void addedNewTask(int pid);
  void removedTask(int pid);
  scheduling_decision schedulingDecision(int& task_to_resign, int& task_to_assign);

};
#endif
