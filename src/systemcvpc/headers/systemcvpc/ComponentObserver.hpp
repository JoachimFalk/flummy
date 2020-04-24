// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2020 Hardware-Software-CoDesign, University of
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

#ifndef _INCLUDED_SYSTEMCVPC_COMPONENTOBSERVER_HPP
#define _INCLUDED_SYSTEMCVPC_COMPONENTOBSERVER_HPP

#include "Attribute.hpp"

#include <boost/intrusive_ptr.hpp>

namespace SystemC_VPC { namespace Extending {

  class ComponentObserverIf;

} } // namespace SystemC_VPC::Extending

namespace SystemC_VPC {

  class Component;
  class ComponentObserver;

  void intrusive_ptr_add_ref(ComponentObserver *p);
  void intrusive_ptr_release(ComponentObserver *p);

  class ComponentObserver {
    typedef ComponentObserver this_type;

    friend void intrusive_ptr_add_ref(this_type *p); // for getImpl
    friend void intrusive_ptr_release(this_type *p); // for getImpl
    friend class Component; // for getImpl
  public:
    typedef boost::intrusive_ptr<this_type>       Ptr;
    typedef boost::intrusive_ptr<this_type const> ConstPtr;

    virtual bool addAttribute(Attribute::Ptr attr) = 0;

    const char *getType() const
      { return type; }
  protected:
    ComponentObserver(int implAdj, const char *type)
      : implAdj(implAdj), type(type) {}

    virtual ~ComponentObserver();
  private:
    Extending::ComponentObserverIf       *getImpl();
    Extending::ComponentObserverIf const *getImpl() const
      { return const_cast<this_type *>(this)->getImpl(); }
  private:
    int         implAdj;
    const char *type;
  };

  ComponentObserver::Ptr createComponentObserver(const char *type, Attribute::Ptr attr = nullptr);

  template <typename OBSERVER>
  typename OBSERVER::Ptr
  createComponentObserver(Attribute::Ptr attr = nullptr) {
    return boost::static_pointer_cast<OBSERVER>(
        createComponentObserver(OBSERVER::Type, attr));
  }

  ComponentObserver::Ptr getComponentObserver(const char *name);

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_COMPONENTOBSERVER_HPP */
