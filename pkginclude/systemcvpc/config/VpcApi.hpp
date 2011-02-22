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
#include "Route.hpp"
#include "Timing.hpp"
#include "VpcTask.hpp"

#include <string>
#include <map>

namespace SystemC_VPC
{

class ScheduledTask;

namespace Config
{

typedef std::map<std::string, Component::Ptr> Components;

Component::Ptr createComponent(std::string name, Scheduler::Type scheduler =
    Scheduler::FCFS);

Component::Ptr getComponent(std::string name);

Route::Ptr createRoute(std::string source, std::string dest, Route::Type type =
    Route::StaticRoute);

Route::Ptr getRoute(std::string source, std::string dest);

bool hasComponent(std::string name);

Components & getComponents();

VpcTask::Ptr getCachedTask(const ScheduledTask & actor); //smoc_actor is a ScheduledTask

VpcTask::Ptr getCachedTask(std::string name);

void setCachedTask(const ScheduledTask * actor, VpcTask::Ptr task);

void setCachedTask(std::string name, VpcTask::Ptr task);

bool hasTask(const ScheduledTask & actor);

bool hasTask(std::string name);

void setPriority(const ScheduledTask & actor, size_t priority);

void ignoreMissingRoutes(bool ignore);

} // namespace Config
} // namespace SystemC_VPC
#endif /* VPC_API_HPP_ */
