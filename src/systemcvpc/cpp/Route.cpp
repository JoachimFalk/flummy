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

#include <systemcvpc/ConfigException.hpp>
#include <systemcvpc/Route.hpp>

#include <iostream>

namespace SystemC_VPC { namespace Config {

//
Hop::Hop(Component::Ptr component) :
  component_(component), transferTiming_(component->getTransferTiming()),
      priority_()
{
}

//
Hop & Hop::setPriority(size_t priority_)
{
  this->priority_ = priority_;
  return *this;
}

Hop & Hop::setTransferTiming(Timing transferTiming_)
{
  this->transferTiming_ = transferTiming_;
  return *this;
}

Component::Ptr Hop::getComponent() const
{
  return component_;
}

size_t Hop::getPriority() const
{
  return priority_;
}

Timing Hop::getTransferTiming() const
{
  return transferTiming_;
}

//
Route::Type Route::parseRouteType(std::string name)
{
  static const std::string B_TRANSPORT = "blocking";
  static const std::string STATIC_ROUTE = "static_route";

  if (name == STATIC_ROUTE) {
    return StaticRoute;
  } else if (name == B_TRANSPORT) {
    return BlockingTransport;
  }

  throw Config::ConfigException("Unknown scheduler \"" + name
      + "\" for component: " + name);
  return StaticRoute;
}

Route::Route(Route::Type type, std::string source, std::string dest) :
  tracing_(false), source_(source), destination_(dest), type_(type),
  routeInterface_(NULL)
{
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
ComponentId Route::getComponentId() const
{
  return this->getSequentialId();
}

std::string Route::getDestination() const
{
  return destination_;
}

std::list<Hop> Route::getHops() const
{
  return hops_;
}

std::string Route::getSource() const
{
  return source_;
}

//
std::string Route::getName() const {
  return "msg_" + getSource() + "_2_" + getDestination();
}

Route::Type Route::getType() const
{
  return type_;
}

void Route::inject(std::string source, std::string destination)
{
  this->source_ = source;
  this->destination_ = destination;
}

RouteInterface::Ptr Route::getRouteInterface() const
{
  //TODO: assert simulation phase
  assert(routeInterface_ != NULL);
  return routeInterface_;
}

//
Hop & Route::addHop(Component::Ptr component)
{
  Hop hop(component);
  hops_.push_back(hop);
  return hops_.back();
}

} } // namespace SystemC_VPC::Config
