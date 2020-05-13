// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2014 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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
 */

#ifndef _INCLUDED_SYSTEMCVPC_VPCEVENT_HPP
#define _INCLUDED_SYSTEMCVPC_VPCEVENT_HPP

#include <CoSupport/SystemC/systemc_support.hpp>

namespace SystemC_VPC {

  /*
   * Event used for messages in order to invalidate them in the Component
   */
  class VPCEvent
  : public CoSupport::SystemC::RefCountEvent {
  public:
    typedef boost::intrusive_ptr<VPCEvent> Ptr;
    typedef VPCEvent this_type;
    //if packet is dropped (e.g. packetloss) dropped is set true
    bool dropped;
  public:
    VPCEvent(bool startNotified = false)
      : CoSupport::SystemC::RefCountEvent(startNotified), dropped(false) {}

    VPCEvent(const Event& e)
        : CoSupport::SystemC::RefCountEvent(e), dropped(false) {}

    CoSupport::SystemC::EventWaiter *reset(CoSupport::SystemC::EventListener *el = NULL) {
      dropped = false;
      return CoSupport::SystemC::RefCountEvent::reset(el);
    }

    void setDropped(bool drop){
      dropped = drop;
    }

    bool getDropped(void){
      return dropped;
    }

    bool isDropped(void){
      return getDropped();
    }
  };

  using ::intrusive_ptr_release;
  using ::intrusive_ptr_add_ref;

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_VPCEVENT_HPP */
