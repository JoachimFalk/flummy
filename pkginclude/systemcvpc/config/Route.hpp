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

#ifndef ROUTE_HPP_
#define ROUTE_HPP_

#include "../datatypes.hpp"
#include "Component.hpp"
#include <boost/shared_ptr.hpp>

#include <string>

namespace SystemC_VPC
{

namespace Config
{

class Hop
{
public:
  Hop(Component::Ptr component);
  void setPriority(size_t priority_);
  void setTransferTiming(Timing transferTiming_);

private:
  Component::Ptr component_;
  //FIXME: forward this to PCB
  Timing transferTiming_;
  size_t priority_;
};

class Route: public SystemC_VPC::SequentiallyIdedObject<ComponentId>
{
public:
  enum Type
  {
    StaticRoute, BlockingTransport
  };
  static Type parseRouteType(std::string name);

  typedef boost::shared_ptr<Route> Ptr;

  Route();
  ComponentId getComponentId() const;
  bool getTracing() const;
  void setTracing(bool tracing_);
  Hop addHop(Component::Ptr component);
  void addTiming(Component::Ptr hop, Timing);
private:
  bool tracing_;
  std::list<Hop> hops_;
  std::map<Component::Ptr, Timing> routeTimings_;
};
} // namespace Config
}

// namespace SystemC_VPC
#endif /* ROUTE_HPP_ */
