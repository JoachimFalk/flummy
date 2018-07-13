// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SYSTEMCVPC_VPCAPI_HPP
#define _INCLUDED_SYSTEMCVPC_VPCAPI_HPP

#include "Component.hpp"
#include "Route.hpp"
#include "Timing.hpp"
#include "VpcTask.hpp"
#include "ScheduledTask.hpp"

#include <string>
#include <map>

namespace SystemC_VPC {

typedef std::map<std::string, Component::Ptr> Components;
typedef std::map<std::string, boost::shared_ptr<DistributionTimingModifier> > Modifiers;

Components &getComponents();

bool hasComponent(std::string name);

Component::Ptr createComponent(std::string name, Scheduler scheduler =
    Scheduler::FCFS);

Component::Ptr getComponent(std::string name);

Route::Ptr createRoute(std::string source, std::string dest, Route::Type type =
    Route::StaticRoute);

Route::Ptr createRoute(const sc_core::sc_port_base * leafPtr, Route::Type type =
    Route::StaticRoute);

Route::Ptr getRoute(std::string source, std::string dest);

Route::Ptr getRoute(const sc_core::sc_port_base * leafPort);

Modifiers & getDistributions();

bool hasDistribution(std::string name);

void createDistribution(std::string name, boost::shared_ptr<DistributionTimingModifier> modifier);




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

void registerComponentWakeup(const char* actor, VPCEvent::Ptr event);

void registerComponentIdle(const char* actor, VPCEvent::Ptr event);

void setCanExec(ScheduledTask & actor, bool canExec);

/*
 * Sets the actor as a PSM actor.
 * The executing state of this actor will always set the component executing state to IDLE
 */
void setActorAsPSM(const char*  name, bool psm);

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_VPCAPI_HPP */