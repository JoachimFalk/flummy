/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 * 
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#ifndef HSCD_VPC_EVENTPAIR_H
#define HSCD_VPC_EVENTPAIR_H

#include <systemc.h>

#include <CoSupport/SystemC/systemc_support.hpp>
#include <systemcvpc/coupling/VPCEvent.hpp>

namespace SystemC_VPC {

  /**
   * Pairing two Events to realize simulation of pipelining!
   *
   * dii is notified if the "first pipeline stage" becomes empty. Possibly the actor can start again.
   * If latency is notified the hole execution time is over.
   */
  struct EventPair{
    Coupling::VPCEvent::Ptr dii;     //data introduction interval
    Coupling::VPCEvent::Ptr latency; //latency

    /**
     * Sloth constructor.
     */
    EventPair( Coupling::VPCEvent::Ptr dii,
        Coupling::VPCEvent::Ptr latency )
      : dii(dii), latency(latency){}
    EventPair() : dii(NULL), latency(NULL){}
  };

} // namespace SystemC_VPC

#endif //HSCD_VPC_EVENTPAIR_H
