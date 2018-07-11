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

#ifndef _INCLUDED_SYSTEMCVPC_ROUTE_HPP
#define _INCLUDED_SYSTEMCVPC_ROUTE_HPP

#include "datatypes.hpp"
#include "Component.hpp"

#include <boost/shared_ptr.hpp>

#include <string>

namespace SystemC_VPC { namespace Detail {

  class Route;
  class StaticRoute;
  template<class ROUTE>
  class RoutePool;

} } // namespace SystemC_VPC::Detail

namespace SystemC_VPC {

class RouteInterface
{
public:
  typedef RouteInterface* Ptr;

  virtual ~RouteInterface(){}
  virtual bool addStream(){return false;}
  virtual bool closeStream(){return false;}
};

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

  Route(Route::Type type, std::string source = "", std::string dest = "");
  ComponentId getComponentId() const;
  bool getTracing() const;
  void setTracing(bool tracing_);
  Hop & addHop(Component::Ptr component);
  void addTiming(Component::Ptr hop, Timing);
  std::string getDestination() const;
  std::list<Hop> getHops() const;
  std::string getSource() const;
  std::string getName() const;
  Type getType() const;
  void inject(std::string source, std::string destination);
  RouteInterface::Ptr getRouteInterface() const;
private:
  friend class Detail::Route;
  friend class Detail::StaticRoute;
  template<class>
  friend class Detail::RoutePool;
  bool tracing_;
  std::list<Hop> hops_;
  std::map<Component::Ptr, Timing> routeTimings_;
  std::string source_;
  std::string destination_;
  Type type_;
  RouteInterface::Ptr routeInterface_;
};

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_ROUTE_HPP */
