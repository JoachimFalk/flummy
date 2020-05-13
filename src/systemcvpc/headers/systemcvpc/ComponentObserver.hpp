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

#ifndef _INCLUDED_SYSTEMCVPC_COMPONENTOBSERVER_HPP
#define _INCLUDED_SYSTEMCVPC_COMPONENTOBSERVER_HPP

#include "ConfigException.hpp"
#include "Attribute.hpp"

#include <boost/intrusive_ptr.hpp>

namespace SystemC_VPC { namespace Extending {

  class ComponentObserverIf;

} } // namespace SystemC_VPC::Extending

namespace SystemC_VPC {

  class Component;
  class ComponentObserver;

  void intrusive_ptr_add_ref(ComponentObserver *p);
  void intrusive_ptr_release(ComponentObserver *p);

  class ComponentObserver {
    typedef ComponentObserver this_type;

    friend void intrusive_ptr_add_ref(this_type *p); // for getImpl
    friend void intrusive_ptr_release(this_type *p); // for getImpl
    friend class Component; // for getImpl
  public:
    typedef boost::intrusive_ptr<this_type>       Ptr;
    typedef boost::intrusive_ptr<this_type const> ConstPtr;

    virtual bool addAttribute(Attribute const &attr) = 0;

    const char *getType() const
      { return type; }
  protected:
    ComponentObserver(int implAdj, const char *type)
      : implAdj(implAdj), type(type) {}

    virtual ~ComponentObserver();
  private:
    Extending::ComponentObserverIf       *getImpl();
    Extending::ComponentObserverIf const *getImpl() const
      { return const_cast<this_type *>(this)->getImpl(); }
  private:
    int         implAdj;
    const char *type;
  };

  class ObserverTypeUnknown: public ConfigException {
  public:
    ObserverTypeUnknown(const char *type);
  };

  class ObserverNameUnknown: public ConfigException {
  public:
    ObserverNameUnknown(const char *type);
  };

  ComponentObserver::Ptr createComponentObserver(
      char const       *type
    , char const       *name
    , Attributes const &attrs = Attributes());

  inline
  ComponentObserver::Ptr createComponentObserver(
      char const       *type
    , Attributes const &attrs = Attributes())
    { return createComponentObserver(type, nullptr, attrs); }

  template <typename OBSERVER>
  typename OBSERVER::Ptr
  inline
  createComponentObserver(
      char const       *name
    , Attributes const &attrs = Attributes())
  {
    return boost::static_pointer_cast<OBSERVER>(
        createComponentObserver(OBSERVER::Type, name, attrs));
  }

  template <typename OBSERVER>
  typename OBSERVER::Ptr
  inline
  createComponentObserver(
      Attributes const &attrs = Attributes())
  {
    return boost::static_pointer_cast<OBSERVER>(
        createComponentObserver(OBSERVER::Type, nullptr, attrs));
  }

  ComponentObserver::Ptr getComponentObserver(const char *name);

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_COMPONENTOBSERVER_HPP */
