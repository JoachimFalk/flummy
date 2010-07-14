/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * EventPair.cpp
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/

#ifndef HSCD_VPC_EVENTPAIR_H
#define HSCD_VPC_EVENTPAIR_H

#include <systemc.h>

#include <CoSupport/SystemC/systemc_support.hpp>

namespace SystemC_VPC {

  /**
   * Pairing two Events to realize simulation of pipelining!
   *
   * dii is notified if the "first pipeline stage" becomes empty. Possibly the actor can start again.
   * If latency is notified the hole execution time is over.
   */
  struct EventPair{
    CoSupport::SystemC::RefCountEventPtr dii;     //data introduction interval
    CoSupport::SystemC::RefCountEventPtr latency; //latency

    /**
     * Sloth constructor.
     */
    EventPair( CoSupport::SystemC::RefCountEventPtr dii,
               CoSupport::SystemC::RefCountEventPtr latency )
      : dii(dii), latency(latency){}
    EventPair() : dii(NULL), latency(NULL){}
  };

} // namespace SystemC_VPC

#endif //HSCD_VPC_EVENTPAIR_H
