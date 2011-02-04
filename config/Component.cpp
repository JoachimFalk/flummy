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

#include <systemcvpc/config/Component.hpp>
#include <systemcvpc/config/Timing.hpp>
#include <systemcvpc/ScheduledTask.hpp>

#include <string>
#include <set>

namespace SystemC_VPC
{

namespace Config
{

//
Component::Component(std::string name, Scheduler::Scheduler scheduler)
{
}
//
void Component::setTransferDelay(sc_core::sc_time transfer_delay)
{
}

//
void Component::setScheduler(Scheduler::Scheduler scheduler)
{
}

//
void Component::addTask(const ScheduledTask & actor)
{

}

//
void Component::setTimings(std::set<Timing> timings)
{

}
} // namespace Config
} // namespace SystemC_VPC

