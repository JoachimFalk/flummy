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
  Hop & setPriority(size_t priority_);
  Hop & setTransferTiming(Timing transferTiming_);
  Component::Ptr getComponent() const;
  size_t getPriority() const;
  Timing getTransferTiming() const;
private:
  Component::Ptr component_;
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

  Route(std::string source, std::string dest, Route::Type type);
  Route(Route::Type type);
  ComponentId getComponentId() const;
  bool getTracing() const;
  void setTracing(bool tracing_);
  Hop & addHop(Component::Ptr component);
  void addTiming(Component::Ptr hop, Timing);
  std::string getDestination() const;
  std::list<Hop> getHops() const;
  std::string getSource() const;
  Type getType() const;
  void inject(std::string source, std::string destination);
private:
  bool tracing_;
  std::list<Hop> hops_;
  std::map<Component::Ptr, Timing> routeTimings_;
  std::string source_;
  std::string destination_;
  Type type_;
};
} // namespace Config
}

// namespace SystemC_VPC
#endif /* ROUTE_HPP_ */