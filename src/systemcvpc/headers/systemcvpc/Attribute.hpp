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

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <boost/shared_ptr.hpp>

#include <deque>
#include <utility>
#include <iostream>
#include "InvalidArgumentException.hpp"

namespace SystemC_VPC{

  class Attribute;

  typedef boost::shared_ptr<Attribute> AttributePtr;
  typedef std::deque<std::pair<std::string, AttributePtr> >  Attributes;
  typedef std::deque<std::pair<std::string, std::string>  >  Parameters;

  class Attribute{
  public:
    Attribute();

    /**
     *
     */
    Attribute( std::string type, std::string value);

    std::pair<std::string, std::string> getNextParameter(size_t pos)
      throw (InvalidArgumentException);

    /**
     *
     */
    std::string getParameter(const std::string type)
      throw (InvalidArgumentException);

    /**
     *
     */
    bool hasParameter(const std::string type);

    void addParameter(std::string type, std::string value);

    std::pair<std::string, AttributePtr > getNextAttribute(size_t pos)
      throw (InvalidArgumentException);

    /**
     *
     */
    AttributePtr getAttribute(const std::string name)
      throw (InvalidArgumentException);

    /**
     *
     */
    bool hasAttribute(const std::string name);

    void addAttribute( std::string type, std::string value);

    void addAttribute( std::string type, AttributePtr att );

    size_t getParameterSize();
    size_t getAttributeSize();

    std::string getValue();
    std::string getType();

    /**
     *
     */
    bool isType(std::string check){
      return this->type==check;
    }

    void setValue(std::string);
    void setType(std::string);

    ~Attribute(){}
  private:
    std::string type;
    std::string value;

    Parameters  parameters;
    Attributes  attributes;
  };
}

#endif

