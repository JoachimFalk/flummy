// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2010 FAU -- Sebastian Graf <sebastian.graf@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_ROUTING_STATIC_HPP
#define _INCLUDED_SYSTEMCVPC_ROUTING_STATIC_HPP

#include "../Route.hpp"

#include <map>

namespace SystemC_VPC { namespace Routing {

class Static: public Route {
  typedef Static this_type;
public:
  typedef boost::intrusive_ptr<this_type>       Ptr;
  typedef boost::intrusive_ptr<this_type const> ConstPtr;

  class Hop;

  static const char *Type;

  Hop  *addHop(Component::Ptr component, Hop *parent = nullptr);
  void  addDest(std::string const &chan, Hop *parent);

  std::set<Static::Hop *>       const &getFirstHops() const;
  std::set<Static::Hop *>       const &getHops() const;

  bool addStream();
  bool closeStream();
protected:
  Static(int implAdj);
};

class Static::Hop {
public:
  size_t  getPriority() const
    { return priority_; }
  Hop    &setPriority(size_t priority_)
    { this->priority_ = priority_; return *this; }

  Timing const &getTransferTiming() const
    { return transferTiming_; }
  Hop    &setTransferTiming(Timing const &transferTiming_)
    { this->transferTiming_ = transferTiming_; return *this; }

  std::list<Hop *> const &getChildHops() const
    { return childHops; }

  Component::Ptr const &getComponent() const
    { return component; }
protected:
  Hop(Component::Ptr const &component);

  std::list<Hop *> childHops;
private:
  Component::Ptr const component;
  Timing               transferTiming_;
  size_t               priority_;
};

} } // namespace SystemC_VPC::Routing

#endif /* _INCLUDED_SYSTEMCVPC_ROUTING_STATIC_HPP */
