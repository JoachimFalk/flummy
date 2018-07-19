// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2018 Hardware-Software-CoDesign, University of
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

#include <systemcvpc/ConfigException.hpp>
#include <systemcvpc/Route.hpp>

#include "detail/Configuration.hpp"
#include "detail/Routing/BlockingTransport.hpp"
#include "detail/Routing/StaticImpl.hpp"
#include "detail/Routing/IgnoreImpl.hpp"

#include <iostream>

#include <string.h>

namespace SystemC_VPC {

Route::Route(const char *type, int implAdj)
  : type(type), implAdj(implAdj)
  , tracing_(false) {}

Detail::AbstractRoute *Route::getImpl() {
  // Pointer magic. Shift our this pointer
  // so that it points to the Detail::AbstractRoute
  // base class of our real implementation.
  return reinterpret_cast<Detail::AbstractRoute *>(
      reinterpret_cast<char *>(this) + implAdj);
}

Route::~Route() {
}

bool Route::getTracing() const
{
  return tracing_;
}

void Route::setTracing(bool tracing_)
{
  this->tracing_ = tracing_;
}
//
RouteId Route::getRouteId() const
{
  return getImpl()->getRouteId();
}

std::string Route::getDestination() const
{
  return destination_;
}

std::string Route::getSource() const
{
  return source_;
}

std::string Route::getName() const {
  return getImpl()->getName();
//return "msg_" + getSource() + "_2_" + getDestination();
}

bool hasRoute(std::string const &name) {
  return Detail::Configuration::getInstance().hasRoute(name).get();
}
bool hasRoute(smoc::SimulatorAPI::PortInInterface  const &port) {
  return Detail::Configuration::getInstance().hasRoute(port.name()).get();
}
bool hasRoute(smoc::SimulatorAPI::PortOutInterface const &port) {
  return Detail::Configuration::getInstance().hasRoute(port.name()).get();
}

Route::Ptr getRoute(std::string const &name) {
  return Detail::Configuration::getInstance().getRoute(name)->getRoute();
}
Route::Ptr getRoute(smoc::SimulatorAPI::PortInInterface  const &port) {
  return Detail::Configuration::getInstance().getRoute(port.name())->getRoute();
}
Route::Ptr getRoute(smoc::SimulatorAPI::PortOutInterface const &port) {
  return Detail::Configuration::getInstance().getRoute(port.name())->getRoute();
}

Route::Ptr createRoute(std::string const &name,
    const char *type) {
  return Detail::Configuration::getInstance().createRoute(name,
    [&name, type]() -> Detail::AbstractRoute::Ptr {
      if (strcmp(type, Detail::Routing::StaticImpl::Type) == 0)
        return new Detail::Routing::StaticImpl(name);
      if (strcmp(type, Detail::Routing::IgnoreImpl::Type) == 0)
        return new Detail::Routing::IgnoreImpl(name);
      else if (strcmp(type, Detail::Routing::BlockingTransport::Type) == 0)
        return new Detail::Routing::BlockingTransport(name);
      else {
        assert(!"WTF?!");
        throw std::runtime_error("Unknown route type!");
      }
    })->getRoute();
}
Route::Ptr createRoute(smoc::SimulatorAPI::PortInInterface  const &port,
    const char *type) {
  Route::Ptr route = createRoute(port.name(), type);
  return route;
}
Route::Ptr createRoute(smoc::SimulatorAPI::PortOutInterface const &port,
    const char *type) {
  Route::Ptr route = createRoute(port.name(), type);
  return route;
}

} // namespace SystemC_VPC
