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

namespace SystemC_VPC
{

class AbstractComponent;

namespace Config
{

typedef std::map<ProcessId, Route::Ptr> Routes;

class Mappings
{
public:
  Mappings();
  // TODO: replace other mapping maps
  // TODO: provide getter, setter, etc.
  static std::map<VpcTask::Ptr, Component::Ptr > & getConfiguredMappings();
  static std::map<Component::Ptr, AbstractComponent * > & getComponents();
  static bool isMapped(VpcTask::Ptr task, Component::Ptr component);

  static void addRoute(ProcessId pid, Route::Ptr route);
  static bool hasRoute(ProcessId pid);
  static Route::Ptr getRoute(ProcessId pid);
};

}

}

#endif /* MAPPINGS_HPP_ */
