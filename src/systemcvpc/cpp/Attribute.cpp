// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
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
#include <iostream>

namespace SystemC_VPC {

  //
  Attribute::Attribute() : type(), value() {}

  //
  Attribute::Attribute( std::string type, std::string value)
    : type(type), value(value){}

  std::pair<std::string, Attribute::Ptr > Attribute::getNextAttribute(size_t pos) {
    if(pos<=attributes.size()) return attributes[pos];
    throw InvalidArgumentException("getNextAttribute");
        
  }

  //
  Attribute::Ptr Attribute::getAttribute(const std::string name) {
    for(unsigned int i=0;
        i<this->getAttributeSize();
        ++i)
      {
        if(attributes[i].first == name)
          return attributes[i].second;
      }
    throw InvalidArgumentException("getAttribute> Unknown Attribute:"
                                       + name);
  }

  //
  bool Attribute::hasAttribute(const std::string name) {
    for(unsigned int i=0;
        i<this->getAttributeSize();
        ++i)
      {
        if(attributes[i].first == name)
          return true;
      }
    return false;
  }

  void Attribute::addAttribute( std::string type, std::string value){
    Attribute::Ptr toadd(new Attribute(type, value));
    attributes.push_back( std::make_pair(type, toadd) );
  }

  void Attribute::addAttribute( std::string type, Attribute::Ptr att ){
    attributes.push_back( std::make_pair(type, att) );
  }

  size_t Attribute::getAttributeSize(){
    return attributes.size();
  }

  std::string Attribute::getValue(){
    return value;
  }

  std::string Attribute::getType(){
    return type;
  }

  IMPL_INTRUSIVE_REFCOUNT_PTR(Attribute)

} // namespace SystemC_VPC::Detail
