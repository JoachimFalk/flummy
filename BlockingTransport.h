/******************************************************************************
 *                        Copyright 2008
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * BlockingTransport.h
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/

#ifndef __INCLUDED__BLOCKINGTRANSPORT__H__
#define __INCLUDED__BLOCKINGTRANSPORT__H__
#include <list>

#include <systemc.h>

#include <CoSupport/SystemC/systemc_support.hpp>

#include <hscd_vpc_AbstractComponent.h>
#include "hscd_vpc_EventPair.h"
#include "hscd_vpc_ProcessControlBlock.h"
#include "Route.h"
#include "Timing.h"

namespace SystemC_VPC{
  /**
   *
   */
  class BlockingTransport :
    public Route,
    public CoSupport::SystemC::Event,
    protected CoSupport::SystemC::EventListener {
  public:

    void compute( Task* task );

    FunctionId getFunctionId(ProcessId pid, std::string function);

    void route( EventPair np );

    void signaled(EventWaiter *e);

    void eventDestroyed(EventWaiter *e);

    void addHop(std::string name, AbstractComponent * hop);

    BlockingTransport( std::string source, std::string dest );

    BlockingTransport( const BlockingTransport & route );

    ~BlockingTransport( );

    const char* getName() const;
  private:
    typedef std::list<AbstractComponent *> Components;

    Components                             components;
    Task*                                  task;
    EventPair                              taskEvents;
    CoSupport::SystemC::Event              dummy;
    std::string                            name;

    
  };
}

#endif // __INCLUDED__BLOCKINGTRANSPORT__H__
