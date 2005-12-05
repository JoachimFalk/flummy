#ifndef HSCD_VPC_SCHEDULERPROXY_H
#define HSCD_VPC_SCHEDULERPROXY_H

#include <map.h>

#include <deque.h>

#include "hscd_vpc_datatypes.h"

#include <systemc.h>

namespace SystemC_VPC{
  class Component;
  class Scheduler;

  class SchedulerProxy : public sc_module{
  public:
  
    SC_CTOR(SchedulerProxy){
      SC_THREAD(schedule_thread);
    }
    sc_event notify_scheduler;

  
    sc_event& getNotifyEvent();
    void registerComponent(Component *comp);
    action_struct* getNextNewCommand(int pid);
    void setScheduler(const char *schedulername);
  
    virtual ~SchedulerProxy();

    /**
     * \brief Set parameter for Component and Scheduler.
     */
    virtual void processAndForwardParameter(char *sType,char *sValue);
  protected:
  
    map<int,action_struct> *open_commands;
    map<int,p_struct*> ready_tasks,running_tasks;
    Component *component;
    Scheduler *scheduler;
    virtual void schedule_thread();

  private:

  };
}
#endif