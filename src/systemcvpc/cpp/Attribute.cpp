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

namespace SystemC_VPC{

  //
  Attribute::Attribute() : type(), value() {}

  //
  Attribute::Attribute( std::string type, std::string value)
    : type(type), value(value){}

  std::pair<std::string, std::string> Attribute::getNextParameter(size_t pos)
    throw(InvalidArgumentException){
    if(pos<=parameters.size()) return parameters[pos];
    throw InvalidArgumentException("getNextParameter");
  }

  //
  std::string Attribute::getParameter(const std::string type)
    throw (InvalidArgumentException){
    for(unsigned int i=0;
        i<this->getParameterSize();
        ++i){
      if(parameters[i].first == type){
        return parameters[i].second;
      }
    }
    throw InvalidArgumentException("getParameter> unknown parameter:"
                                       + type);
  }

  //
  bool Attribute::hasParameter(const std::string type){
    for(unsigned int i=0;
        i<this->getParameterSize();
        ++i){
      if(parameters[i].first == type){
        return true;
      }
    }
    return false;
  }

  void Attribute::addParameter(std::string type,std::string value){
    this->parameters.push_back( std::make_pair(type, value) );
  }

  std::pair<std::string, AttributePtr > Attribute::getNextAttribute(size_t pos)
    throw(InvalidArgumentException){
    if(pos<=attributes.size()) return attributes[pos];
    throw InvalidArgumentException("getNextAttribute");
        
  }

  //
  AttributePtr Attribute::getAttribute(const std::string name)
    throw(InvalidArgumentException){
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
    AttributePtr toadd(new Attribute(type, value));
    attributes.push_back( std::make_pair(type, toadd) );
  }

  void Attribute::addAttribute( std::string type, AttributePtr att ){
    attributes.push_back( std::make_pair(type, att) );
  }

  size_t Attribute::getParameterSize(){
    return parameters.size();
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

  void Attribute::setValue(std::string newValue){
    value=newValue;
  }

  void Attribute::setType(std::string newType){
    type=newType;
  }
}