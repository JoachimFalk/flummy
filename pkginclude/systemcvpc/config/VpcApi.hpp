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

#ifndef VPC_API_HPP_
#define VPC_API_HPP_

#include "Component.hpp"
#include "VpcTask.hpp"
#include "Timing.hpp"

#include <string>
#include <map>

namespace SystemC_VPC
{

class ScheduledTask;

namespace Config
{
Component::Ptr createComponent(std::string name, Scheduler::Scheduler = Scheduler::FCFS);

Component::Ptr getComponent(std::string name);

bool hasComponent(std::string name);

VpcTask::Ptr createTask(const ScheduledTask & actor); //smoc_actor is a ScheduledTask

void setPriority(const ScheduledTask & actor, size_t priority);

} // namespace Config
} // namespace SystemC_VPC
#endif /* VPC_API_HPP_ */
