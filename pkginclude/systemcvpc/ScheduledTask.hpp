/*
 * ScheduledTask.h
 *
 *  Created on: Nov 17, 2010
 *      Author: streubuehr
 */

#ifndef SCHEDULEDTASK_HPP_
#define SCHEDULEDTASK_HPP_

#include <cstddef>

namespace SystemC_VPC
{

typedef size_t ProcessId;

class Delayer;

class ScheduledTask
{
public:
  ScheduledTask();
  virtual ~ScheduledTask();
  void setDelayer(Delayer *component);
  void setPid(ProcessId pid);
  ProcessId getPid() const;
  void notifyActivation(bool active);
private:
  Delayer *component;
  ProcessId pid;
};

}

#endif /* SCHEDULEDTASK_HPP_ */
