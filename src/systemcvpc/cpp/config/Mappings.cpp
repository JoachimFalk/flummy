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

#include "Mappings.hpp"
#include "../BlockingTransport.hpp"
#include "../RoutePool.hpp"
#include "../StaticRoute.hpp"
#include "../ProcessControlBlock.hpp"

namespace SystemC_VPC
{

namespace Config
{

//
std::map<VpcTask::Ptr, Component::Ptr>& Mappings::getConfiguredMappings()
{
  static std::map<VpcTask::Ptr, Component::Ptr> configuredMappings;
  return configuredMappings;
}

//
std::map<Component::Ptr, AbstractComponent *> & Mappings::getComponents()
{
  static std::map<Component::Ptr, AbstractComponent *> components;
  return components;
}

//
bool Mappings::isMapped(VpcTask::Ptr task, Component::Ptr component)
{
  std::map<VpcTask::Ptr, Component::Ptr>& mappings =
      Mappings::getConfiguredMappings();
  return (mappings.find(task) != mappings.end()) && mappings[task] == component;
}

namespace Routing
{

class Impl
{
public:
  void add(const ProcessId pid, Route::Ptr route)
  {
    assert(!has(pid));
    routes[pid] = route;
  }

  void add(const sc_core::sc_port_base * leafPort, Route::Ptr route)
  {
    assert(!has(leafPort));
    routesByPort[leafPort] = route;
  }

  void set(const ProcessId pid, Route::Ptr route)
  {
    routes[pid] = route;
  }

  void set(const sc_core::sc_port_base * leafPort, Route::Ptr route)
  {
    routesByPort[leafPort] = route;
  }

  bool has(const ProcessId pid) const
  {
    return routes.find(pid) != routes.end();
  }

  bool has(const sc_core::sc_port_base * leafPort) const
  {
    return routesByPort.find(leafPort) != routesByPort.end();
  }

  Route::Ptr get(const ProcessId pid) const
  {
    assert(has(pid));
    return routes.find(pid)->second;
  }

  Route::Ptr get(const sc_core::sc_port_base * leafPort) const
  {
    assert(has(leafPort));
    return routesByPort.find(leafPort)->second;
  }

private:
  Routes routes;
  RoutesByPort routesByPort;
};

Impl& impl()
{
  static Impl i;
  return i;
}

//
void add(const ProcessId pid, Route::Ptr route)
{
  impl().add(pid, route);
}

//
void add(const sc_core::sc_port_base *leafPort, Route::Ptr route)
{
  impl().add(leafPort, route);
}

//
bool has(const ProcessId pid)
{
  return impl().has(pid);
}

//
bool has(const sc_core::sc_port_base *leafPort)
{
  return impl().has(leafPort);
}

//
Route::Ptr get(const ProcessId pid)
{
  return impl().get(pid);
}

//
Route::Ptr get(const sc_core::sc_port_base *leafPort)
{
  return impl().get(leafPort);
}

//
void set(const ProcessId pid, Route::Ptr route)
{
  impl().set(pid, route);
}

//
void set(const sc_core::sc_port_base *leafPort, Route::Ptr route)
{
  impl().set(leafPort, route);
}

//
SystemC_VPC::Route * create(Config::Route::Ptr configuredRoute)
{
  SystemC_VPC::Route * route;
  switch (configuredRoute->getType()) {
    default:
    case Config::Route::StaticRoute:
      {
        RoutePool<StaticRoute> * staticRoute = new RoutePool<StaticRoute> (configuredRoute);
        route = staticRoute;
        staticRoute->setRouteInterface(&(staticRoute->getPrototype()));
      }
      break;
    case Config::Route::BlockingTransport:
      route = new RoutePool<BlockingTransport> (configuredRoute);
      break;
  }
  std::list<Hop> hops = configuredRoute->getHops();
  for (std::list<Hop>::const_iterator iter = hops.begin(); iter != hops.end(); ++iter) {
    Config::Component::Ptr component = iter->getComponent();
    AbstractComponent * c = Config::Mappings::getComponents()[component];
    route->addHop(component->getName(), c);

    ProcessControlBlock *pcb = c->createPCB(route->getName());
    pcb->configure(route->getName(), false);
    pcb->setTiming(iter->getTransferTiming());
    pcb->setPriority(iter->getPriority());
  }
  return route;
}

} // namespace Routing
} // namespace Config
} // namespace SystemC_VPC

