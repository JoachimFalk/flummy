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

#ifndef _INCLUDED_SYSTEMCVPC_ATTRIBUTE_HPP
#define _INCLUDED_SYSTEMCVPC_ATTRIBUTE_HPP

#include "InvalidArgumentException.hpp"

#include <CoSupport/SmartPtr/RefCount.hpp>
#include <CoSupport/SmartPtr/intrusive_refcount_ptr.hpp>

#include <boost/intrusive_ptr.hpp>

#include <list>
#include <map>
#include <string>

#include <deque>

namespace SystemC_VPC {

  class Attribute;

  class Attributes {
  private:
    typedef std::list<Attribute> AL;
  public:
    typedef Attribute           value_type;
    typedef value_type         &reference;
    typedef value_type const   &const_reference;

    typedef AL::iterator        iterator;
    typedef AL::const_iterator  const_iterator;

    typedef AL::difference_type difference_type;
    typedef AL::size_type       size_type;
  public:

    iterator       begin() noexcept
      { return attributeList.begin(); }
    const_iterator begin() const noexcept
      { return attributeList.begin(); }
    const_iterator cbegin() const noexcept
      { return attributeList.begin(); }

    iterator       end() noexcept
      { return attributeList.end(); }
    const_iterator end() const noexcept
      { return attributeList.end(); }
    const_iterator cend() const noexcept
      { return attributeList.end(); }

    bool      empty() const noexcept
      { return attributeList.empty(); }
    size_type size() const noexcept
      { return attributeList.size(); }
    size_type max_size() const noexcept
      { return attributeList.max_size(); }

    reference       front()
      { return attributeList.front(); }
    const_reference front() const
      { return attributeList.front(); }

    reference       back()
      { return attributeList.back(); }
    const_reference back() const
      { return attributeList.back(); }

    iterator insert(const_iterator iter, const_reference val);


  private:
    typedef std::map<std::string, iterator> AM;

    AL attributeList;
    AM attributeMap;
  };

  class Attribute;

  DECL_INTRUSIVE_REFCOUNT_PTR(Attribute, PAttribute)

  class Attribute
    : private CoSupport::SmartPtr::RefCount
  {
    typedef Attribute this_type;

    friend void intrusive_ptr_add_ref(this_type *);
    friend void intrusive_ptr_release(this_type *);
  public:
    typedef boost::intrusive_ptr<this_type>       Ptr;
    typedef boost::intrusive_ptr<this_type const> ConstPtr;

    Attribute();

    /**
     *
     */
    Attribute( std::string type, std::string value);

    std::pair<std::string, Attribute::Ptr > getNextAttribute(size_t pos);

    /**
     *
     */
    Attribute::Ptr getAttribute(const std::string name);

    /**
     *
     */
    bool hasAttribute(const std::string name);

    void addAttribute( std::string type, std::string value);

    void addAttribute( std::string type, Attribute::Ptr att );

    size_t getAttributeSize();

    std::string const &getValue() const noexcept
      { return value; }
    std::string const &getType() const noexcept
    { return type; }

    /**
     *
     */
    bool isType(std::string check){
      return this->type==check;
    }

    ~Attribute(){}
  private:
    std::string type;
    std::string value;

    typedef std::deque<std::pair<std::string, PAttribute> >  Attributes;

    Attributes  attributes;
  };

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_ATTRIBUTE_HPP */

