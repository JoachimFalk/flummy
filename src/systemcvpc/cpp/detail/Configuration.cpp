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

#include "Configuration.hpp"

#include "Director.hpp"
#include "Routing/BlockingTransport.hpp"
#include "Routing/StaticImpl.hpp"

#include <CoSupport/String/QuotedString.hpp>

#include <sstream>

namespace SystemC_VPC { namespace Detail {

  using CoSupport::String::QuotedString;

  Configuration::Configuration() {
  }

  Configuration &Configuration::getInstance() {
    static Configuration conf;
    return conf;
  }

  /// This will return the map of all components.
  Components const &Configuration::getComponents() const
    { return components; }
  /// This might return a nullptr.
  AbstractComponent::Ptr Configuration::hasComponent(std::string const &name) const {
    Components::const_iterator iter = components.find(name);
    return iter != components.end()
        ? iter->second
        : nullptr;
  }
  /// This will not return a nullptr. If the component does not
  /// exists, then a ConfigException will be thrown.
  AbstractComponent::Ptr Configuration::getComponent(std::string const &name) const {
    Components::const_iterator iter = components.find(name);
    if (iter != components.end())
      return iter->second;
    std::stringstream msg;
    msg << "Cannot get component " << QuotedString(name) << " before creation. Use createComponent first.";
    throw ConfigException(msg.str().c_str());
  }
  /// This will create a component of the given name. A factory
  /// function has to be supplied. If a component with an identical
  /// name already exists, a ConfigException will be thrown.
  AbstractComponent::Ptr Configuration::createComponent(std::string const &name,
      std::function<AbstractComponent::Ptr ()> constr) {
    std::pair<Components::iterator, bool> status =
        components.insert(Components::value_type(name, nullptr));
    if (status.second) {
      try {
        status.first->second = constr();
        return status.first->second;
      } catch (...) {
        components.erase(status.first);
        throw;
      }
    }
    std::stringstream msg;
    msg << "Multiple creation of component " << QuotedString(name) << " is not supported. Use getComponent instead.";
    throw ConfigException(msg.str().c_str());
  }

  /// This will return the map of all tasks.
  VpcTasks const &Configuration::getVpcTasks() const
    { return vpcTasks; }
  /// This might return a nullptr.
  VpcTask::Ptr Configuration::hasVpcTask(std::string const &name) const {
    VpcTasks::const_iterator iter = vpcTasks.find(name);
    return iter != vpcTasks.end()
        ? iter->second
        : nullptr;
  }
  /// This will not return a nullptr. If the task does not
  /// exists, then a ConfigException will be thrown.
  VpcTask::Ptr Configuration::getVpcTask(std::string const &name) const {
    VpcTasks::const_iterator iter = vpcTasks.find(name);
    if (iter != vpcTasks.end())
      return iter->second;
    std::stringstream msg;
    msg << "Cannot get VPC task " << QuotedString(name) << " before creation. Use createVpcTask first.";
    throw ConfigException(msg.str().c_str());
  }
  /// This will create a task of the given name. A factory
  /// function has to be supplied. If a task with an identical
  /// name already exists, a ConfigException will be thrown.
  VpcTask::Ptr Configuration::createVpcTask(std::string const &name,
      std::function<VpcTask::Ptr ()> constr) {
    std::pair<VpcTasks::iterator, bool> status =
        vpcTasks.insert(VpcTasks::value_type(name, nullptr));
    if (status.second) {
      try {
        status.first->second = constr();
        return status.first->second;
      } catch (...) {
        vpcTasks.erase(status.first);
        throw;
      }
    }
    std::stringstream msg;
    msg << "Multiple creation of task " << QuotedString(name) << " is not supported. Use getVpcTask instead.";
    throw ConfigException(msg.str().c_str());
  }

  /// This will return the map of all routes.
  Routes const &Configuration::getRoutes() const
    { return routes; }

  /// This might return a nullptr.
  AbstractRoute::Ptr Configuration::hasRoute(std::string const &name) const {
    Routes::const_iterator iter = routes.find(name);
    return iter != routes.end()
        ? iter->second
        : nullptr;
  }
  /// This will not return a nullptr. If the route does not
  /// exists, then a ConfigException will be thrown.
  AbstractRoute::Ptr Configuration::getRoute(std::string const &name) const {
    Routes::const_iterator iter = routes.find(name);
    if (iter != routes.end())
      return iter->second;
    std::stringstream msg;
    msg << "Cannot get route " << QuotedString(name) << " before creation. Use createRoute first.";
    throw ConfigException(msg.str().c_str());
  }
  /// This will create a route of the given name. A factory
  /// function has to be supplied. If a route with an identical
  /// name already exists, a ConfigException will be thrown.
  AbstractRoute::Ptr Configuration::createRoute(std::string const &name,
      std::function<AbstractRoute::Ptr ()> constr) {
    std::pair<Routes::iterator, bool> status =
        routes.insert(Routes::value_type(name, nullptr));
    if (status.second) {
      try {
        status.first->second = constr();
        return status.first->second;
      } catch (...) {
        routes.erase(status.first);
        throw;
      }
    }
    std::stringstream msg;
    msg << "Multiple creation of route " << QuotedString(name) << " is not supported. Use getRoute instead.";
    throw ConfigException(msg.str().c_str());
  }

  /// This will return the map of all routes.
  TimingModifiers const &Configuration::getTimingModifiers() const
    { return timingModifiers; }
  /// This might return a nullptr.
  TimingModifier::Ptr Configuration::hasTimingModifier(std::string const &name) const {
    TimingModifiers::const_iterator iter = timingModifiers.find(name);
    return iter != timingModifiers.end()
        ? iter->second
        : nullptr;
  }
  /// This will not return a nullptr. If the route does not
  /// exists, then a ConfigException will be thrown.
  TimingModifier::Ptr Configuration::getTimingModifier(std::string const &name) const {
    TimingModifiers::const_iterator iter = timingModifiers.find(name);
    if (iter != timingModifiers.end())
      return iter->second;
    std::stringstream msg;
    msg << "Cannot get timing modifier " << QuotedString(name) << " before creation. Use createTimingModifier first.";
    throw ConfigException(msg.str().c_str());
  }
  /// This will create a route of the given name. A factory
  /// function has to be supplied. If a route with an identical
  /// name already exists, a ConfigException will be thrown.
  TimingModifier::Ptr Configuration::createTimingModifier(std::string const &name,
      std::function<TimingModifier::Ptr ()> constr) {
    std::pair<TimingModifiers::iterator, bool> status =
        timingModifiers.insert(TimingModifiers::value_type(name, nullptr));
    if (status.second) {
      try {
        status.first->second = constr();
        return status.first->second;
      } catch (...) {
        timingModifiers.erase(status.first);
        throw;
      }
    }
    std::stringstream msg;
    msg << "Multiple creation of timing modifier " << QuotedString(name) << " is not supported. Use getTimingModifier instead.";
    throw ConfigException(msg.str().c_str());
  }

} } // namespace SystemC_VPC::Detail
