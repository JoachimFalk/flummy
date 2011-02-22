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

namespace SystemC_VPC
{

namespace Config
{

Mappings::Mappings()
{
}

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

Routes & getRoutes()
{
  static Routes routes;
  return routes;
}

//
void Mappings::addRoute(ProcessId pid, Route::Ptr route)
{
  getRoutes()[pid] = route;
}

//
bool Mappings::hasRoute(ProcessId pid)
{
  return getRoutes().find(pid) != getRoutes().end();
}

//
Route::Ptr Mappings::getRoute(ProcessId pid)
{
  assert(hasRoute(pid));
  return getRoutes()[pid];
}
} // namespace Config
} // namespace SystemC_VPC

