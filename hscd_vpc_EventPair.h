/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_EventPair.cpp
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
#include <systemc_support.hpp>

namespace SystemC_VPC {

  /**
   * Pairing two Events to realise simulation of pipelinig!
   *
   * dii is notified if the "first pipeline stage" becomes empty. Possibly the actor can start again.
   * If latency is notified the hole execution time is over.
   */
  struct EventPair{
    CoSupport::SystemC::Event* dii;     //data introduction intervall
    CoSupport::SystemC::Event* latency; //latency

    /**
     * Sloth constructor.
     */
    EventPair( CoSupport::SystemC::Event* dii, CoSupport::SystemC::Event* latency ) : dii(dii), latency(latency){}
  };

} // namespace SystemC_VPC

#endif //HSCD_VPC_EVENTPAIR_H
