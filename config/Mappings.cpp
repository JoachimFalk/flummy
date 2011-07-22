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

#include "Mappings.hpp"

#include <systemcvpc/BlockingTransport.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/RoutePool.hpp>
#include <systemcvpc/StaticRoute.hpp>

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

  void add(const sc_port_base * leafPort, Route::Ptr route)
  {
    assert(!has(leafPort));
    routesByPort[leafPort] = route;
  }

  void set(const ProcessId pid, Route::Ptr route)
  {
    routes[pid] = route;
  }

  void set(const sc_port_base * leafPort, Route::Ptr route)
  {
    routesByPort[leafPort] = route;
  }

  bool has(const ProcessId pid) const
  {
    return routes.find(pid) != routes.end();
  }

  bool has(const sc_port_base * leafPort) const
  {
    return routesByPort.find(leafPort) != routesByPort.end();
  }

  Route::Ptr get(const ProcessId pid) const
  {
    assert(has(pid));
    return routes.find(pid)->second;
  }

  Route::Ptr get(const sc_port_base * leafPort) const
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
void add(const sc_port_base *leafPort, Route::Ptr route)
{
  impl().add(leafPort, route);
}

//
bool has(const ProcessId pid)
{
  return impl().has(pid);
}

//
bool has(const sc_port_base *leafPort)
{
  return impl().has(leafPort);
}

//
Route::Ptr get(const ProcessId pid)
{
  return impl().get(pid);
}

//
Route::Ptr get(const sc_port_base *leafPort)
{
  return impl().get(leafPort);
}

//
void set(const ProcessId pid, Route::Ptr route)
{
  impl().set(pid, route);
}

//
void set(const sc_port_base *leafPort, Route::Ptr route)
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
      route = new RoutePool<StaticRoute> (configuredRoute);
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

    ProcessControlBlockPtr pcb = c->createPCB(Director::getProcessId(route->getName()));
    pcb->configure(route->getName(), false, true);
    pcb->setTiming(iter->getTransferTiming());
    pcb->setPriority(iter->getPriority());

  }
  return route;
}

} // namespace Routing
} // namespace Config
} // namespace SystemC_VPC

