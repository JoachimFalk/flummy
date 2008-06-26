#ifndef HSCD_VPC_TASK_H_
#define HSCD_VPC_TASK_H_

#include <sstream>

#include "FastLink.h"

namespace SystemC_VPC {
  class Task{
  public:
    Task(){}

    Task(FastLink link, EventPair pair)
      :  pid(link.process),
      fid (link.func),
      blockEvent(pair) {}

    std::string getName(){
      return "FIXME";
    }

    ProcessId  pid;
    FunctionId fid;
    EventPair  blockEvent;
  };
}
#endif // HSCD_VPC_TASK_H_
