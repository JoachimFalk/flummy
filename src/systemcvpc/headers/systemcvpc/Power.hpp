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

  Power(std::string const &v);
  Power(char const *v);
};

std::ostream &operator <<(std::ostream &out, Power const &p);
std::istream &operator >>(std::istream &in, Power &p);

} // namespace SystemC_VPC

#endif // _INCLUDED_SYSTEMCVPC_POWER_HPP
