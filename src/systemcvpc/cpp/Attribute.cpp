// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2020 Hardware-Software-CoDesign, University of
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

#include <systemcvpc/Attribute.hpp>

namespace SystemC_VPC {

  Attributes::iterator Attributes::insert(const_iterator iter, const_reference val) {
    Attributes::iterator retval = attributeList.insert(iter, val);

    std::pair<AM::iterator, bool> status =
        attributeMap.insert(AM::value_type(val.getType(), retval));


    return retval;
  }

  /**
   * Returns an iterator to an attribute with the given type.
   * If no such attribute is present, returns end(). If multiple
   * such attributes are present, returns an iterator to one of
   * them. Incrementing the returned iterator will, in general,
   * NOT point to the next attribute with the given type if multiple
   * of them are present!
   */
  Attributes::iterator Attributes::find(std::string const &type) {
    AM::iterator iter = attributeMap.find(type);
    return iter !=  attributeMap.end()
        ? iter->second
        : attributeList.end();
  }

  /**
   * Returns attribute with given type. If no such attribute
   * is present, throws InvalidArgumentException. If multiple
   * such attributes are present, returns one of them.
   */
  Attributes::reference Attributes::operator[](std::string const &type) {
    AM::iterator iter = attributeMap.find(type);
    if (iter != attributeMap.end())
      return *iter->second;
    throw InvalidArgumentException("Unknown Attribute: " + type);
  }

  Attribute::Attribute(
      std::string const &type
    , std::string const &value)
    : type(type), value(value) {}
  Attribute::Attribute(
      std::string const &type
    , Attributes const &attrs)
    : type(type), attributes(attrs) {}
  Attribute::Attribute(
      std::string const &type
    , std::string const &value
    , Attributes const &attrs)
    : type(type), value(value), attributes(attrs) {}

} // namespace SystemC_VPC::Detail
