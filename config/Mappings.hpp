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

typedef std::map<const ProcessId, Route::Ptr> Routes;
typedef std::map<const sc_port_base *, Route::Ptr> RoutesByPort;

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
  void add(const ProcessId pid, Route::Ptr route);
  void add(const sc_port_base * leafPort, Route::Ptr route);

  void set(const ProcessId pid, Route::Ptr route);
  void set(const sc_port_base * leafPort, Route::Ptr route);

  bool has(const ProcessId pid);
  bool has(const sc_port_base * leafPort);

  Route::Ptr get(const ProcessId pid);
  Route::Ptr get(const sc_port_base * leafPort);
  SystemC_VPC::Route * create(Config::Route::Ptr configuredRoute);

} // namespace Routing

} // namespace Config
} // namespace SystemC_VPC

#endif /* MAPPINGS_HPP_ */
