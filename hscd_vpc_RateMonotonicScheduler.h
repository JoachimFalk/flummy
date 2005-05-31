
#ifndef HSCD_VPC_RATEMONOTONICSCHEDULER_H
#define HSCD_VPC_RATEMONOTONICSCHEDULER_H
#include "systemc.h"
#include "hscd_vpc_Scheduler.h"
#include "hscd_vpc_datatypes.h"
#include <map.h>
#include <vector.h>
namespace SystemC_VPC{
  class Component;

  class RateMonotonicScheduler : public Scheduler, public sc_module{
  public:
    SC_CTOR(RateMonotonicScheduler){
      SC_THREAD(schedule_thread);
      //   cerr << "Scheduler: dms"<<endl;
    }
    //  map<int,p_struct> getReadyTasks();
    // map<int,p_struct> getRunningTasks();
    void registerComponent(Component *comp);
    sc_event& getNotifyEvent();
    sc_event notify_scheduler;


    void schedule_thread();
    action_struct* getNextNewCommand(int pid);

    virtual ~RateMonotonicScheduler();
  protected:
    map<int,action_struct> *open_commands;
    map<int,p_struct> ready_tasks,running_tasks;
    Component *component;
  };
}
#endif
