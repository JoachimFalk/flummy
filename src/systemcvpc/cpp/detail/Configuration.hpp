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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_CONFIGURATION_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_CONFIGURATION_HPP

#include <systemcvpc/VpcTask.hpp>
#include <systemcvpc/TimingModifier.hpp>

#include "AbstractComponent.hpp"
#include "AbstractRoute.hpp"

#include <smoc/SimulatorAPI/TaskInterface.hpp>
#include <smoc/SimulatorAPI/PortInterfaces.hpp>
#include <smoc/SimulatorAPI/FiringRuleInterface.hpp>

#include <map>
#include <functional>

namespace SystemC_VPC { namespace Detail {

typedef std::map<std::string, AbstractComponent::Ptr>   Components;
typedef std::map<std::string, VpcTask::Ptr>             VpcTasks;
typedef std::map<std::string, AbstractRoute::Ptr>       Routes;
typedef std::map<std::string, TimingModifier::Ptr>      TimingModifiers;

class Configuration {
  typedef Configuration this_type;

  typedef smoc::SimulatorAPI::TaskInterface         TaskInterface;
  typedef smoc::SimulatorAPI::FiringRuleInterface   FiringRuleInterface;
  typedef smoc::SimulatorAPI::PortInInterface       PortInInterface;
  typedef smoc::SimulatorAPI::PortOutInterface      PortOutInterface;
public:
  /**
   * \brief Access to singleton Configuration.
   */
  static this_type &getInstance();

  /// This will return the map of all components.
  Components const &getComponents() const;
  /// This might return a nullptr.
  AbstractComponent::Ptr hasComponent(std::string const &name) const;
  /// This will not return a nullptr. If the component does not
  /// exists, then a ConfigException will be thrown.
  AbstractComponent::Ptr getComponent(std::string const &name) const;
  /// This will create a component of the given name. A factory
  /// function has to be supplied. If a component with an identical
  /// name already exists, a ConfigException will be thrown.
  AbstractComponent::Ptr createComponent(std::string const &name,
      std::function<AbstractComponent::Ptr ()> constr);

  /// This will return the map of all tasks.
  VpcTasks const &getVpcTasks() const;
  /// This might return a nullptr.
  VpcTask::Ptr hasVpcTask(std::string const &name) const;
  /// This will not return a nullptr. If the task does not
  /// exists, then a ConfigException will be thrown.
  VpcTask::Ptr getVpcTask(std::string const &name) const;
  /// This will create a task of the given name. A factory
  /// function has to be supplied. If a task with an identical
  /// name already exists, a ConfigException will be thrown.
  VpcTask::Ptr createVpcTask(std::string const &name,
      std::function<VpcTask::Ptr ()> constr);
  /// This is triggered from SysteMoC via SystemCVPCSimulator to
  /// register a task, i.e., a SysteMoC actor. For each registered
  /// SysteMoC actor there must be a corresponding createVpcTask
  /// triggered by building the VPC configuration, e.g., by VPCBuilder.
  void registerTask(
      TaskInterface                          *task,
      std::list<FiringRuleInterface *> const &firingRules);

  /// This will return the map of all routes.
  Routes const &getRoutes() const;
  /// This might return a nullptr.
  AbstractRoute::Ptr hasRoute(std::string const &name) const;
  /// This will not return a nullptr. If the route does not
  /// exists, then a ConfigException will be thrown.
  AbstractRoute::Ptr getRoute(std::string const &name) const;
  /// This will create a route of the given name. A factory
  /// function has to be supplied. If a route with an identical
  /// name already exists, a ConfigException will be thrown.
  AbstractRoute::Ptr createRoute(std::string const &name,
      std::function<AbstractRoute::Ptr ()> constr);
  /// This is triggered from SysteMoC via SystemCVPCSimulator to
  /// register a route, i.e., a SysteMoC port. For each registered
  /// SysteMoC port there must be a corresponding createRoute
  /// triggered by building the VPC configuration, e.g., by VPCBuilder.
  void registerRoute(PortInInterface *port);
  /// This is triggered from SysteMoC via SystemCVPCSimulator to
  /// register a route, i.e., a SysteMoC port. For each registered
  /// SysteMoC port there must be a corresponding createRoute
  /// triggered by building the VPC configuration, e.g., by VPCBuilder.
  void registerRoute(PortOutInterface *port);

  /// This will return the map of all timing modifiers.
  TimingModifiers const &getTimingModifiers() const;
  /// This might return a nullptr.
  TimingModifier::Ptr hasTimingModifier(std::string const &name) const;
  /// This will not return a nullptr. If the timing modifier does not
  /// exists, then a ConfigException will be thrown.
  TimingModifier::Ptr getTimingModifier(std::string const &name) const;
  /// This will create a timing modifier of the given name. A factory
  /// function has to be supplied. If a timing modifier with an identical
  /// name already exists, a ConfigException will be thrown.
  TimingModifier::Ptr createTimingModifier(std::string const &name,
      std::function<TimingModifier::Ptr ()> constr);

  /// Do a consistency check and finalize stuff for start of simulation.
  void finalize();
private:
  Configuration();

  struct RegisteredTask {
    RegisteredTask(
        TaskInterface                          *task,
        std::list<FiringRuleInterface *> const &firingRules)
      : task(task), firingRules(firingRules) {}

    // The registered task
    TaskInterface *task;
    // Its firing rules.
    std::list<FiringRuleInterface *> const &firingRules;
  };

  typedef std::map<std::string, RegisteredTask> RegisteredTasks;

  typedef boost::variant<
      PortInInterface *, PortOutInterface *>     RegisteredRoute;
  typedef std::map<std::string, RegisteredRoute> RegisteredRoutes;

  /// Marker if finalize has run and registerXXX and createXXX calls
  /// are no longer allowed.
  bool                  finalized;

  Components            components;
  /// These are the created tasks
  VpcTasks              vpcTasks;
  /// These are the registered tasks
  RegisteredTasks       registeredTasks;
  /// These are the created routes
  Routes                routes;
  /// These are the registered routes
  RegisteredRoutes      registeredRoutes;
  TimingModifiers       timingModifiers;
};

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_CONFIGURATION_HPP */
