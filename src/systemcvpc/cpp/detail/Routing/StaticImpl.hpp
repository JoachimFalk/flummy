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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_ROUTING_STATICIMPL_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_ROUTING_STATICIMPL_HPP

#include <systemcvpc/Routing/Static.hpp>
#include <systemcvpc/EventPair.hpp>

#include "../Director.hpp"
#include "../AbstractRoute.hpp"
#include "../ProcessControlBlock.hpp"

#include <CoSupport/SystemC/systemc_support.hpp>

#include <systemc>

#include <list>

namespace SystemC_VPC { namespace Detail { namespace Routing {

  /**
   *
   */
  class StaticImpl
    : public AbstractRoute
    , public SystemC_VPC::Routing::Static
  {
    typedef StaticImpl this_type;
  public:
    typedef boost::intrusive_ptr<this_type>       Ptr;
    typedef boost::intrusive_ptr<this_type> const ConstPtr;

    StaticImpl(std::string const &name);

    ///
    /// Handle interfaces for SystemC_VPC::Route
    ///

    // For resolving ambiguity
    using AbstractRoute::getName;
    using AbstractRoute::getRouteId;

    ///
    /// Handle interfaces for SystemC_VPC::Routing::Static
    ///

    Hop  &addHop(Component::Ptr component);

    std::list<Hop> const &getHops() const;

    bool addStream();
    bool closeStream();

    ///
    /// Handle interfaces for AbstractRoute
    ///

    Route *getRoute();

    void   start(size_t quantitiy, VPCEvent::Ptr finishEvent);

    ///
    /// Other stuff
    ///

//  void compute( TaskInstance* task );
//
//  void route( EventPair np );
//
//  void signaled(EventWaiter *e);
//
//  void eventDestroyed(EventWaiter *e);

    ~StaticImpl();
  private:
    struct HopImpl {
      HopImpl(Hop &hop)
        : hop(hop), pcb(nullptr) {}

      Hop                 &hop;
      ProcessControlBlock *pcb;
    };

    typedef std::list<Hop>      Hops;
    Hops                        hops;

    typedef std::list<HopImpl>  HopImpls;
    HopImpls                    hopImpls;

    struct MessageInstance {
      MessageInstance(
          StaticImpl    *staticImpl,
          size_t         quantitiy,
          VPCEvent::Ptr  finishEvent);
    private:
      void startHop();
      void finishHop();
    private:
      StaticImpl         *staticImpl;
      size_t              quantitiy;
      VPCEvent::Ptr       finishEvent;
      HopImpls::iterator  currHop;
    };
  };

  inline
  void intrusive_ptr_add_ref(StaticImpl *p) {
    ::intrusive_ptr_add_ref(p);
  }

  inline
  void intrusive_ptr_release(StaticImpl *p) {
    ::intrusive_ptr_release(p);
  }

} } } // namespace SystemC_VPC::Detail::Routing

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_ROUTING_STATICIMPL_HPP */
