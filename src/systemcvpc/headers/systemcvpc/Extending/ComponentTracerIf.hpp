// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_EXTENDING_COMPONENTTRACERIF_HPP
#define _INCLUDED_SYSTEMCVPC_EXTENDING_COMPONENTTRACERIF_HPP

#include "../Component.hpp"
#include "../ComponentTracer.hpp"

#include "ComponentObserverIf.hpp"

#include <functional>

namespace SystemC_VPC { namespace Extending {

class ComponentTracerIf
  : public ComponentObserverIf {
  typedef ComponentTracerIf this_type;
public:

  ComponentTracer       *getComponentTracer() {
    return static_cast<ComponentTracer *>(
        this->getComponentObserver());
  }
  ComponentTracer const *getComponentTracer() const
    { return const_cast<this_type *>(this)->getComponentTracer(); }

protected:
  ComponentTracerIf(int facadeAdj, size_t rc, size_t rt, size_t rti)
    : ComponentObserverIf(facadeAdj, rc, rt, rti) {}

  typedef std::function<
      this_type *(Attributes)> FactoryFunction;

  static void registerTracer(
      const char      *type,
      FactoryFunction  factory);
};

} } // namespace SystemC_VPC::Extending

#endif /* _INCLUDED_SYSTEMCVPC_EXTENDING_COMPONENTTRACERIF_HPP */
