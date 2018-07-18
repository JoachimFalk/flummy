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
#include <systemcvpc/Routing/Static.hpp>

#include "detail/Configuration.hpp"
#include "detail/Routing/BlockingTransport.hpp"
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

Static::Hop *Static::getFirstHop() {
  return Routing::getImpl(this)->getFirstHop();
}

std::map<Component::Ptr, Static::Hop> const &Static::getHops() const {
  return reinterpret_cast<std::map<Component::Ptr, Static::Hop> const &>
    (Routing::getImpl(this)->getHops());
}

bool Static::addStream()
  { return Routing::getImpl(this)->addStream(); }
bool Static::closeStream()
  { return Routing::getImpl(this)->closeStream(); }

} } // namespace SystemC_VPC::Routing
