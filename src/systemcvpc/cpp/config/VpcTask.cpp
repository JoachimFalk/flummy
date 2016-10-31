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
VpcTask::VpcTask(ScheduledTask & actor) :
  actor_(&actor), priority_(0), psm_(false)
{
}

//
VpcTask::VpcTask() :
  actor_(NULL), priority_(0), psm_(false)
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

const Component::Ptr  VpcTask::getComponent() const
{
	return component_;
}

//
void VpcTask::inject(ScheduledTask * actor)
{
  actor_ = actor;
}

void VpcTask::setActorAsPSM(bool psm)
{
	psm_ = psm;
}

bool VpcTask::isPSM()
{
	return this->psm_;
}

} // namespace Config
} // namespace SystemC_VPC
