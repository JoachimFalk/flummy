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

#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/Delayer.hpp>

namespace SystemC_VPC
{

ScheduledTask::ScheduledTask() :
  component(NULL)
{
}

ScheduledTask::~ScheduledTask()
{
}
void ScheduledTask::setDelayer(Delayer *component)
{
  this->component = component;
}

Delayer* ScheduledTask::getDelayer()
{
	return this->component;
}

void ScheduledTask::notifyActivation(bool active)
{
  if (component != NULL) { // FALLBACKMODE
    component->notifyActivation(this, active);
  }
}

void ScheduledTask::setPid(ProcessId pid)
{
  this->pid = pid;
}

ProcessId ScheduledTask::getPid() const
{
  return this->pid;
}

} // namespace SystemC_VPC
