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

#ifndef __INCLUDED__STATICROUTE__H__
#define __INCLUDED__STATICROUTE__H__
#include <list>

#include <systemc.h>

#include <CoSupport/SystemC/systemc_support.hpp>
#include <CoSupport/Tracing/TracingFactory.hpp>

#include "config/Route.hpp"
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
  class StaticRoute :
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

    void setPool(RoutePool<StaticRoute> * pool);

    const ComponentList& getHops() const;

    StaticRoute( Config::Route::Ptr configuredRoute );

    StaticRoute( const StaticRoute & route );

    ~StaticRoute( );
  private:
    typedef std::list<AbstractComponent *> Components;

    Components                             components;
    Task*                                  task;
    EventPair                              taskEvents;
    CoSupport::SystemC::RefCountEventPtr   dummyDii;
    CoSupport::SystemC::RefCountEventPtr   routeLat;
    Components::iterator                   nextHop;
    RoutePool<StaticRoute>                *pool;
  };
}

#endif // __INCLUDED__STATICROUTE__H__
