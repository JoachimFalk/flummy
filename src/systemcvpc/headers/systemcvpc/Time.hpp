// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2020 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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
