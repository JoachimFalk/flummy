// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
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

#ifndef _INCLUDED_SYSTEMCVPC_TIME_HPP
#define _INCLUDED_SYSTEMCVPC_TIME_HPP

#include <boost/units/make_scaled_unit.hpp>
#include <boost/units/quantity.hpp>
//#include <boost/units/systems/si/energy.hpp>
#include <boost/units/systems/si/time.hpp>
//#include <boost/units/systems/si/force.hpp>
//#include <boost/units/systems/si/length.hpp>
//#include <boost/units/systems/si/electric_potential.hpp>
//#include <boost/units/systems/si/current.hpp>
//#include <boost/units/systems/si/resistance.hpp>
//#include <boost/units/systems/si/io.hpp>
#include <boost/units/static_rational.hpp>

#include <iostream>

namespace SystemC_VPC {

typedef boost::units::make_scaled_unit<boost::units::si::time,
  boost::units::scale<10, boost::units::static_rational<-12> > >::type picosecond_unit;

picosecond_unit picosecond;

class Time
  : public boost::units::quantity<picosecond_unit, int64_t>
{
  typedef Time this_type;
public:
  typedef boost::units::quantity<picosecond_unit, int64_t> base_type;

  template <typename U, typename T>
  Time(boost::units::quantity<U, T> v)
    : base_type(v) {}
  Time(this_type const &v)
    : base_type(v) {}
};

std::ostream &operator <<(std::ostream &out, Time const &t);

}  // namespace SystemC_VPC

#endif // _INCLUDED_SYSTEMCVPC_TIME_HPP
