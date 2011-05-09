/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 * 
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
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
  Delayer* getDelayer();
  void setPid(ProcessId pid);
  ProcessId getPid() const;
  void notifyActivation(bool active);
private:
  Delayer *component;
  ProcessId pid;
};

}

#endif /* SCHEDULEDTASK_HPP_ */
