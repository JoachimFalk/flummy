// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2011 FAU -- Jens Gladigau <jens.gladigau@cs.fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 Thomas Russ <tr.thomas.russ@googlemail.com>
 *   2012 Tim Grocki <grocki@codesign.informatik.uni-erlangen.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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
 */

#include <systemcvpc/ConfigException.hpp>
#include <systemcvpc/Route.hpp>

#include "detail/Configuration.hpp"
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

void setIgnoreMissingRoutes(bool ignore) {
  Detail::Configuration::getInstance().setIgnoreMissingRoutes(ignore);
}

bool getIgnoreMissingRoutes() {
  return Detail::Configuration::getInstance().getIgnoreMissingRoutes();
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
