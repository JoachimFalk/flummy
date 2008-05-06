/******************************************************************************
 *                        Copyright 2008
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * StaticRoute.h
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/

#ifndef __INCLUDED__STATICROUTE__H__
#define __INCLUDED__STATICROUTE__H__
#include <list>

#include <systemc.h>

#include <cosupport/systemc_support.hpp>

#include <hscd_vpc_AbstractComponent.h>
#include "hscd_vpc_EventPair.h"
#include "hscd_vpc_ProcessControlBlock.h"
#include "Timing.h"

namespace SystemC_VPC{
  /**
   *
   */
  class StaticRoute :
    public Delayer,
    public CoSupport::SystemC::Event,
    protected CoSupport::SystemC::EventListener {
  public:

    /**
     *
     */
    void compute( ProcessControlBlock *pcb );

    void route( EventPair np );

    void signaled(EventWaiter *e);

    void eventDestroyed(EventWaiter *e);

    void addHop(std::string name, AbstractComponent * hop);

    StaticRoute( std::string source, std::string dest );

    StaticRoute( const StaticRoute & route );

    ~StaticRoute( );

    const char* getName() const;
  private:
    typedef std::list<AbstractComponent *> Components;

    Components                             components;
    ProcessControlBlock *                  task;
    EventPair                              taskEvents;
    CoSupport::SystemC::Event              dummy;
    std::string                            name;

    
  };
}

#endif // __INCLUDED__STATICROUTE__H__
