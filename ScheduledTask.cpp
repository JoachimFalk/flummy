/*
 * ScheduledTask.cpp
 *
 *  Created on: Nov 17, 2010
 *      Author: streubuehr
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
