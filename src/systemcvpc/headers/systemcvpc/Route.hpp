// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2020 Hardware-Software-CoDesign, University of
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

#include <smoc/SimulatorAPI/PortInterfaces.hpp>

#include <CoSupport/SmartPtr/RefCountObject.hpp>

#include <boost/noncopyable.hpp>

#include <string>

namespace SystemC_VPC { namespace Detail {

  class AbstractRoute;

} } // namespace SystemC_VPC::Detail

namespace SystemC_VPC {

class Route
  : private boost::noncopyable
  , public CoSupport::SmartPtr::RefCountObject
{
  typedef Route this_type;
public:
  typedef boost::intrusive_ptr<this_type>       Ptr;
  typedef boost::intrusive_ptr<this_type> const ConstPtr;

  RouteId getRouteId() const;

  bool getTracing() const;
  void setTracing(bool tracing_);

  std::string getDestination() const;
  std::string getSource() const;
  std::string getName() const;

  const char *getType() const
    { return type; }
protected:
  Route(const char *type, int implAdj);

  ~Route();
private:
  Detail::AbstractRoute       *getImpl();
  Detail::AbstractRoute const *getImpl() const
    { return const_cast<this_type *>(this)->getImpl(); }
private:
  const char *type;
  int         implAdj;
  bool tracing_;
  std::string source_;
  std::string destination_;
};

/// Usually, for all SysteMoC ports, i.e., messages, there must be a
/// route created via createRoute. However, if true is given to the
/// setIgnoreMissingRoutes function. Then, the routing of messages where
/// no route has been created is ignored, i.e., zero communication
/// overhead is assumed in the VPC simulation.
void setIgnoreMissingRoutes(bool ignore);
/// Getter returning if VPC will ignore missing routes.
bool getIgnoreMissingRoutes();

/// Check if a route has been created via createRoute.
bool hasRoute(std::string const &name);
/// Check if a route has been created via createRoute.
bool hasRoute(smoc::SimulatorAPI::PortInInterface  const &port);
/// Check if a route has been created via createRoute.
bool hasRoute(smoc::SimulatorAPI::PortOutInterface const &port);

/// This will not return a nullptr. If the route does not
/// exists, then a ConfigException will be thrown.
Route::Ptr getRoute(std::string const &name);
/// This will not return a nullptr. If the route does not
/// exists, then a ConfigException will be thrown.
Route::Ptr getRoute(smoc::SimulatorAPI::PortInInterface  const &port);
/// This will not return a nullptr. If the route does not
/// exists, then a ConfigException will be thrown.
Route::Ptr getRoute(smoc::SimulatorAPI::PortOutInterface const &port);

/// This will create a route for the given message, i.e.,
/// SysteMoC port, and type of route.
Route::Ptr createRoute(std::string const &name,
    const char *type);
/// This will create a route for the given message, i.e.,
/// SysteMoC port, and type of route.
Route::Ptr createRoute(smoc::SimulatorAPI::PortInInterface  const &port,
    const char *type);
/// This will create a route for the given message, i.e.,
/// SysteMoC port, and type of route.
Route::Ptr createRoute(smoc::SimulatorAPI::PortOutInterface const &port,
    const char *type);

/// This will create a route for the given message, i.e.,
/// SysteMoC port, and type of route.
template <typename ROUTE>
typename ROUTE::Ptr
createRoute(std::string const &name) {
  return boost::static_pointer_cast<ROUTE>(
      createRoute(name, ROUTE::Type));
}
/// This will create a route for the given message, i.e.,
/// SysteMoC port, and type of route.
template <typename ROUTE>
typename ROUTE::Ptr
createRoute(smoc::SimulatorAPI::PortInInterface  const &port) {
  return boost::static_pointer_cast<ROUTE>(
      createRoute(port, ROUTE::Type));
}
/// This will create a route for the given message, i.e.,
/// SysteMoC port, and type of route.
template <typename ROUTE>
typename ROUTE::Ptr
createRoute(smoc::SimulatorAPI::PortOutInterface const &port) {
  return boost::static_pointer_cast<ROUTE>(
      createRoute(port, ROUTE::Type));
}

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_ROUTE_HPP */
