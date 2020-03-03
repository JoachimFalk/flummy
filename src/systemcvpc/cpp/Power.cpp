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

#include <systemcvpc/Power.hpp>

#include <boost/units/systems/si/io.hpp>

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cctype>
#include <cerrno>
#include <cstring>

namespace SystemC_VPC {

#define UNITCASE_VALUE(x) (((x) >= 'a' && ((x) <= 'z')) \
    ? static_cast<size_t>((x) - 'a') \
    : static_cast<size_t>((x) - 'A' + 'z'-'a'+1))
#define UNITCASE_BASE static_cast<size_t>('z'-'a'+'Z'-'A'+2)

#define UNITCASE1(x1)                   UNITCASE_VALUE(x1)
#define UNITCASE2(x1,x2)                (UNITCASE_VALUE(x1)+UNITCASE1(x2)*UNITCASE_BASE)
#define UNITCASE3(x1,x2,x3)             (UNITCASE_VALUE(x1)+UNITCASE2(x2,x3)*UNITCASE_BASE)
#define UNITCASE4(x1,x2,x3,x4)          (UNITCASE_VALUE(x1)+UNITCASE3(x2,x3,x4)*UNITCASE_BASE)
#define UNITCASE5(x1,x2,x3,x4,x5)       (UNITCASE_VALUE(x1)+UNITCASE4(x2,x3,x4,x5)*UNITCASE_BASE)

  static Power::quantity_type convert(std::string const &v, bool *error = nullptr) {
    size_t len = v.size();

    // Strip white space from end
    while (len > 0 && std::isspace(v[len-1]))
      --len;
    do {
      size_t unit    = 0;
      size_t unitLen = 0;
      for (int index = len-1; index > 0 && unitLen < 5; --index) {
        if (v[index] >= 'a' && v[index] <= 'z') {
          ++unitLen;
          unit *= 'z'-'a'+'Z'-'A'+2;
          unit += v[index]-'a';
        } else if (v[index] >= 'A' && v[index] <= 'Z') {
          ++unitLen;
          unit *= 'z'-'a'+'Z'-'A'+2;
          unit += v[index]-'A' + 'z'-'a'+1;
        } else
          break;
      }
      if (!unitLen)
        break;
      double scale;
      switch (unit) {
        case UNITCASE1('W'):     scale = 1e0;   break;
        case UNITCASE2('m','W'): scale = 1e-3;  break;
        case UNITCASE2('u','W'): scale = 1e-6;  break;
        case UNITCASE2('n','W'): scale = 1e-9;  break;
        case UNITCASE2('p','W'): scale = 1e-12; break;
        default:                 scale = 0;     break;
      }
      if (!scale)
        break;

      size_t valueEnd = len - unitLen;
      // Strip white space from value end
      while (valueEnd > 0 && std::isspace(v[valueEnd-1]))
        --valueEnd;

      errno = 0;
      char *endptr = nullptr;
      double value = strtod(v.c_str(), &endptr);

      if (errno || !endptr)
        break;
      if (static_cast<size_t>(endptr - v.c_str()) != valueEnd)
        break;

      if (error)
        *error = false;

      return  Power::quantity_type(value*scale*boost::units::si::watt);
    } while(false);
    if (!error)
      throw std::runtime_error(std::string("Can't convert ")+v+" into a power value!");
    *error = true;
    return Power::quantity_type();
  }

  Power::Power(std::string const &v)
    : base_type(convert(v)) {}
  Power::Power(char const *v)
    : base_type(convert(v)) {}

  std::ostream &operator <<(std::ostream &out, Power const &p) {
    std::stringstream msg;
    msg << boost::units::engineering_prefix
        << static_cast<Power::quantity_type const &>(p);
    return out << msg.rdbuf()->str();
  }

  std::istream &operator >>(std::istream &in, Power &p) {
    if (!in.good())
      return in;
    std::string value;
    in >> value;
    if (value.empty()) {
      in.setstate(std::ios::badbit);
      return in;
    }
    // Do we need a unit?
    if (std::isdigit(value[value.size()-1])) {
      std::string unit;
      in >> unit;
      value += " ";
      value += unit;
    }
    bool error;
    p = convert(value, &error);
    if (error)
      in.setstate(std::ios::badbit);
    return in;
  }

} // namespace SystemC_VPC
