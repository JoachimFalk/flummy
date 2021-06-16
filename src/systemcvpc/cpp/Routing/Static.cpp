// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
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
#include <systemcvpc/Routing/Static.hpp>

#include "detail/Configuration.hpp"
#include "detail/Routing/StaticImpl.hpp"

#include <iostream>

#include <stddef.h>

namespace SystemC_VPC { namespace Routing {

//
Static::Hop::Hop(Component::Ptr const &component)
  : component(component)
  , transferTiming_(component->getTransferTiming())
  , priority_(0) {}

const char *Static::Type = "StaticRoute";

Static::Static(int implAdj)
  : Route(Type, implAdj) {}

Static::Hop *Static::addHop(Component::Ptr component, Hop *parent) {
  return Routing::getImpl(this)->addHop(component, parent);
}

void         Static::addDest(std::string const &chan, Hop *parent) {
  return Routing::getImpl(this)->addDest(chan, parent);
}

std::set<Static::Hop *> const &Static::getFirstHops() const {
  return reinterpret_cast<std::set<Static::Hop *> const &>
    (Routing::getImpl(this)->getFirstHops());
}

std::set<Static::Hop *> const &Static::getHops() const {
  return reinterpret_cast<std::set<Static::Hop *> const &>
    (Routing::getImpl(this)->getHops());
}

bool Static::addStream()
  { return Routing::getImpl(this)->addStream(); }
bool Static::closeStream()
  { return Routing::getImpl(this)->closeStream(); }

} } // namespace SystemC_VPC::Routing
