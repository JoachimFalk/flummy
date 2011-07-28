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

#ifndef __INCLUDED__BLOCKINGTRANSPORT__H__
#define __INCLUDED__BLOCKINGTRANSPORT__H__
#include <list>
#include <utility>

#include <systemc.h>

#include <CoSupport/SystemC/systemc_support.hpp>

#include "AbstractComponent.hpp"
#include "EventPair.hpp"
#include "ProcessControlBlock.hpp"
#include "RouteImpl.hpp"
#include "Timing.hpp"

namespace SystemC_VPC{
  template<class ROUTE>
  class RoutePool;

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

    void setPool(RoutePool<BlockingTransport> * pool);

    const ComponentList& getHops() const;

    BlockingTransport( Config::Route::Ptr configuredRoute );

    BlockingTransport( const BlockingTransport & route );

    ~BlockingTransport( );
  private:
    void resetHops();
    void resetLists();

  private:
    enum Phase {
      LOCK_ROUTE,
      COMPUTE_ROUTE
    };
    typedef std::list<std::pair<AbstractComponent *, Task *> > Components;

    Components                             hopList;
    Components                             lockList;
    Components::iterator                   nextHop;
    ComponentList                          components;

    // a rout is either input (read) or output (write)
    bool                                   isWrite;

    Task*                                  task;
    EventPair                              taskEvents;
    Coupling::VPCEvent::Ptr                dummyDii;
    Coupling::VPCEvent::Ptr                routeLat;
    RoutePool<BlockingTransport>          *pool;

    // blocking transport has two phases:
    // - lock the route
    // - apply the route
    Phase                                  phase;

    
  };
}

#endif // __INCLUDED__BLOCKINGTRANSPORT__H__
