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
#include <systemc.h>

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

  virtual sc_time getNextReleaseTime() {
    return sc_time_stamp();
  }

  void setActive(bool a);

  bool getActive(){
    return active;
  }

private:
  Delayer *component;
  ProcessId pid;
  bool active;
};

}

#endif /* SCHEDULEDTASK_HPP_ */
