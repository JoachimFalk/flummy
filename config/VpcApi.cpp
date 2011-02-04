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

#include <systemcvpc/config/VpcApi.hpp>
#include <systemcvpc/config/Component.hpp>

#include <string>
#include <iostream>

namespace SystemC_VPC
{
namespace Config
{

static std::map<std::string, Component::Ptr> components;
static std::map<const ScheduledTask *, VpcTask::Ptr> vpcTasks;

//
Component::Ptr createComponent(std::string name, Scheduler::Scheduler scheduler)
{
  return Component::Ptr(new Component(name, scheduler));
}

//
Component::Ptr getComponent(std::string name)
{
  return createComponent(name);
}

//
VpcTask::Ptr createTask(const ScheduledTask & actor)
{
  // the pid is not yet injected
  //std::cerr << "createTask: " << actor.getPid() << std::endl;
  VpcTask::Ptr vpcTask(new VpcTask(actor));
  vpcTasks[&actor] = vpcTask;
  return vpcTask;
}

//
void setPriority(const ScheduledTask & actor, size_t priority)
{
}
} // namespace Config
} // namespace SystemC_VPC
