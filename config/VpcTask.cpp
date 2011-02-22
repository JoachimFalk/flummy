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

#include <systemcvpc/config/VpcTask.hpp>
#include <systemcvpc/config/Component.hpp>
#include <systemcvpc/config/ConfigException.hpp>
#include <systemcvpc/ScheduledTask.hpp>

namespace SystemC_VPC
{

namespace Config
{

//
VpcTask::VpcTask(const ScheduledTask & actor) :
  actor_(&actor)
{
}

//
VpcTask::VpcTask() :
  actor_(NULL)
{
}

//
void VpcTask::mapTo(Component::Ptr component)
{
  component->addTask(*actor_);
}

//
void VpcTask::setPriority(size_t priority)
{
  priority_ = priority;
}

//
size_t VpcTask::getPriority() const
{
  return priority_;
}
//
const ScheduledTask * VpcTask::getActor() const
{
  return actor_;
}

//
void VpcTask::inject(const ScheduledTask * actor)
{
  actor_ = actor;
}

} // namespace Config
} // namespace SystemC_VPC
