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

#ifndef MAPPINGS_HPP_
#define MAPPINGS_HPP_

#include <systemcvpc/config/Component.hpp>
#include <systemcvpc/config/Route.hpp>
#include <systemcvpc/config/VpcTask.hpp>

#include <map>

#include <systemc>

namespace SystemC_VPC
{

class AbstractComponent;
class Route;

namespace Config
{

typedef std::map<ProcessId, Route::Ptr> Routes;
typedef std::map<sc_port_base *, Route::Ptr> RoutesByPort;

namespace Mappings
{
  // TODO: replace other mapping maps
  // TODO: provide getter, setter, etc.
  std::map<VpcTask::Ptr, Component::Ptr > & getConfiguredMappings();
  std::map<Component::Ptr, AbstractComponent * > & getComponents();
  bool isMapped(VpcTask::Ptr task, Component::Ptr component);

} // namespace Mappings

namespace Routing
{
  void add(ProcessId pid, Route::Ptr route);
  void add(sc_port_base * leafPort, Route::Ptr route);

  void set(ProcessId pid, Route::Ptr route);
  void set(sc_port_base * leafPort, Route::Ptr route);

  bool has(ProcessId pid);
  bool has(sc_port_base * leafPort);

  Route::Ptr get(ProcessId pid);
  Route::Ptr get(sc_port_base * leafPort);
  SystemC_VPC::Route * create(Config::Route::Ptr configuredRoute);

} // namespace Routing

} // namespace Config
} // namespace SystemC_VPC

#endif /* MAPPINGS_HPP_ */
