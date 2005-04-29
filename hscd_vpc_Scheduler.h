#include "systemc.h"
#ifndef HSCD_VPC_SCHEDULER_H
#define HSCD_VPC_SCHEDULER_H
#include "hscd_vpc_datatypes.h"


class Component;
class Scheduler{
 public:
  virtual void schedule(int iprocess)=0;
  virtual sc_event& getNotifyEvent()=0;  
  virtual void registerComponent(Component *comp)=0;
  virtual action_struct* getNextNewCommand(int pid)=0;

};
#endif
