// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2011 Thomas Russ <tr.thomas.russ@googlemail.com>
 *   2012 FAU -- Sebastian Graf <sebastian.graf@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_COMPONENTTRACER_HPP
#define _INCLUDED_SYSTEMCVPC_COMPONENTTRACER_HPP

#include "ConfigException.hpp"
#include "ComponentObserver.hpp"

#include <boost/intrusive_ptr.hpp>

namespace SystemC_VPC {

  class ComponentTracer
    : public ComponentObserver
  {
    typedef ComponentTracer this_type;

//  friend class Component; // for getImpl
  public:
    typedef boost::intrusive_ptr<this_type>       Ptr;
    typedef boost::intrusive_ptr<this_type const> ConstPtr;

  protected:
    ComponentTracer(int implAdj, const char *type)
      : ComponentObserver(implAdj, type) {}
  private:
//  Extending::ComponentObserverIf       *getImpl();
//  Extending::ComponentObserverIf const *getImpl() const
//    { return const_cast<this_type *>(this)->getImpl(); }
  };

  class TracerTypeUnknown: public ConfigException {
  public:
    TracerTypeUnknown(const char *type);
  };

  class TracerNameUnknown: public ConfigException {
  public:
    TracerNameUnknown(const char *type);
  };

  ComponentTracer::Ptr createComponentTracer(
      char const       *type
    , char const       *name
    , Attributes const &attrs = Attributes());

  inline
  ComponentTracer::Ptr createComponentTracer(
      char const       *type
    , Attributes const &attrs = Attributes())
    { return createComponentTracer(type, nullptr, attrs); }

  template <typename TRACER>
  typename TRACER::Ptr
  inline
  createComponentTracer(
      char const       *name
    , Attributes const &attrs = Attributes())
  {
    return boost::static_pointer_cast<TRACER>(
        createComponentTracer(TRACER::Type, name, attrs));
  }

  template <typename TRACER>
  typename TRACER::Ptr
  inline
  createComponentTracer(
      Attributes const &attrs = Attributes())
  {
    return boost::static_pointer_cast<TRACER>(
        createComponentTracer(TRACER::Type, nullptr, attrs));
  }

  ComponentTracer::Ptr getComponentTracer(const char *name);

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_COMPONENTTRACER_HPP */
