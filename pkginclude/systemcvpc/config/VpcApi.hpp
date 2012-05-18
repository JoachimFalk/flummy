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
#include "../Delayer.hpp"

#include <string>
#include <map>

namespace SystemC_VPC
{

class ScheduledTask;

namespace Config
{

typedef std::map<std::string, Component::Ptr> Components;
typedef std::map<std::string, boost::shared_ptr<DistributionTimingModifier> > Modifiers;

void createDistribution(std::string name, boost::shared_ptr<DistributionTimingModifier> modifier);

Component::Ptr createComponent(std::string name, Scheduler::Type scheduler =
    Scheduler::FCFS);

Component::Ptr getComponent(std::string name);

Route::Ptr createRoute(std::string source, std::string dest, Route::Type type =
    Route::StaticRoute);

Route::Ptr createRoute(const sc_port_base * leafPtr, Route::Type type =
    Route::StaticRoute);

Route::Ptr getRoute(std::string source, std::string dest);

Route::Ptr getRoute(const sc_port_base * leafPort);

bool hasDistribution(std::string name);

bool hasComponent(std::string name);

Modifiers & getDistributions();

Components & getComponents();

VpcTask::Ptr getCachedTask(ScheduledTask & actor); //smoc_actor is a ScheduledTask

VpcTask::Ptr getCachedTask(std::string name);

void setCachedTask(const ScheduledTask * actor, VpcTask::Ptr task);

void setCachedTask(std::string name, VpcTask::Ptr task);

bool hasTask(const ScheduledTask & actor);

bool hasTask(std::string name);

void setPriority(ScheduledTask & actor, size_t priority);

void ignoreMissingRoutes(bool ignore);

ComponentInterface* getTaskComponentInterface(ScheduledTask & actor);

void changePowerMode(ScheduledTask & actor,std::string powermode);

bool hasWaitingOrRunningTasks(ScheduledTask & actor);

/*
 * Sets the actor as a PSM actor.
 * The executing state of this actor will always set the component executing state to IDLE
 */
void setActorAsPSM(const char*  name, bool psm);

} // namespace Config
} // namespace SystemC_VPC
#endif /* VPC_API_HPP_ */
