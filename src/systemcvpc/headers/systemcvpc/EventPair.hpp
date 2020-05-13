// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2014 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_EVENTPAIR_HPP
#define _INCLUDED_SYSTEMCVPC_EVENTPAIR_HPP

#include <systemc>

#include "VPCEvent.hpp"

namespace SystemC_VPC {

  /**
   * Pairing two Events to realize simulation of pipelining!
   *
   * dii is notified if the "first pipeline stage" becomes empty. Possibly the actor can start again.
   * If latency is notified the hole execution time is over.
   */
  struct EventPair{
    VPCEvent::Ptr dii;     //data introduction interval
    VPCEvent::Ptr latency; //latency

    /**
     * Sloth constructor.
     */
    EventPair( VPCEvent::Ptr dii,
        VPCEvent::Ptr latency )
      : dii(dii), latency(latency){}
    EventPair() : dii(NULL), latency(NULL){}
  };

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_EVENTPAIR_HPP */
