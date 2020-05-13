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

#ifndef _INCLUDED_SYSTEMCVPC_POWER_HPP
#define _INCLUDED_SYSTEMCVPC_POWER_HPP

#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/power.hpp>
//#include <boost/units/systems/si/time.hpp>
//#include <boost/units/systems/si/force.hpp>
//#include <boost/units/systems/si/length.hpp>
//#include <boost/units/systems/si/electric_potential.hpp>
//#include <boost/units/systems/si/current.hpp>
//#include <boost/units/systems/si/resistance.hpp>
//#include <boost/units/static_rational.hpp>

#include <iostream>

namespace SystemC_VPC {

class Power
  : public boost::units::quantity<boost::units::si::power>
{
  typedef Power this_type;
  typedef boost::units::quantity<boost::units::si::power> base_type;
public:
  typedef base_type quantity_type;

  Power() {}
  Power(this_type const &v)
    : base_type(v) {}
  template <typename U, typename T>
  Power(boost::units::quantity<U, T> v)
    : base_type(v) {}

  explicit Power(std::string const &v);
  explicit Power(char const *v);
};

std::ostream &operator <<(std::ostream &out, Power const &p);
std::istream &operator >>(std::istream &in, Power &p);

} // namespace SystemC_VPC

#endif // _INCLUDED_SYSTEMCVPC_POWER_HPP
