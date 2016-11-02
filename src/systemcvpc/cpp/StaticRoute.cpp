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

#include <CoSupport/Tracing/TracingFactory.hpp>
#include <CoSupport/Tracing/PtpTracer.hpp>

#include <systemcvpc/StaticRoute.hpp>
#include <systemcvpc/RoutePool.hpp>
#include <systemcvpc/Director.hpp>

#include <systemcvpc/debug_config.hpp>
// if compiled with DBG_STATIC_ROUTE create stream and include debug macros
#ifdef DBG_STATIC_ROUTE
#include <CoSupport/Streams/DebugOStream.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include <systemcvpc/debug_on.hpp>
#else
  #include <systemcvpc/debug_off.hpp>
#endif

namespace SystemC_VPC {

  //
  void StaticRoute::compute( Task* _task ) {
    // reset hop list
    nextHop = components.begin();

    task = _task;
    taskEvents = task->getBlockEvent();
    if(components.empty()){
      taskEvents.dii->notify();
      //taskEvents.latency->notify();
      Director::getInstance().signalLatencyEvent(task);
      this->pool->free(this);
      return;
    }

    this->traceStart();

    this->route( EventPair(taskEvents.dii, routeLat) );
  }

  //
  void StaticRoute::route( EventPair np ){
    if(nextHop != components.end()){
      //EventPair np(pcb->getBlockEvent().dii, pcb->getBlockEvent().latency);
      Task *newTask =
        Director::getInstance().allocateTask(task->getProcessId());
      newTask->setTimingScale(task->getTimingScale());
      newTask->setBlockEvent(np);
      newTask->setFunctionIds(task->getFunctionIds());
      DBG_OUT("route on: " << components.front()->getName() << endl);
      (*nextHop)->compute(newTask);
      ++nextHop;
      if(newTask->getBlockEvent().latency->isDropped()){
        taskEvents.latency->setDropped(newTask->getBlockEvent().latency->isDropped());
        task->setBlockEvent(taskEvents);
        nextHop = components.end();
        routeLat->reset();
        this->traceStop();
        Director::getInstance().signalLatencyEvent(task);
         this->pool->free(this);
       }
    } else {
      this->traceStop();

      assert(dummyDii == np.dii);
      Director::getInstance().signalLatencyEvent(task);
      this->pool->free(this);
    }
  }

  //
  void StaticRoute::signaled(EventWaiter *e) {
    if(e->isActive()){
      DBG_OUT("signaled @ " << sc_time_stamp() << endl);
      if(task->getBlockEvent().latency->isDropped()){
        nextHop = components.end();
      }
      routeLat->reset();
      route( EventPair(dummyDii, routeLat) );
    }
  }

  //
  void StaticRoute::eventDestroyed(EventWaiter *e){
    DBG_OUT("eventDestroyed" << endl);
  }

  //
  void StaticRoute::addHop(std::string name, AbstractComponent * hop){
    components.push_back(hop);
  }

  //
  void StaticRoute::setPool(RoutePool<StaticRoute> * pool)
  {
    this->pool = pool;
  }

  //
  const ComponentList& StaticRoute::getHops() const {
    return components;
  }

  //
  StaticRoute::StaticRoute( Config::Route::Ptr configuredRoute ) :
    Route(configuredRoute),
    dummyDii(new Coupling::VPCEvent()),
    routeLat(new Coupling::VPCEvent())
  {
    routeLat->addListener(this);
    configuredRoute->routeInterface_ = this;
  }

  //
  StaticRoute::StaticRoute( const StaticRoute & route ) :
    Route(route),
    components(),
    task(route.task),
    taskEvents(route.taskEvents),
    dummyDii(new Coupling::VPCEvent()),
    routeLat(new Coupling::VPCEvent())
  {
    routeLat->addListener(this);
    for(Components::const_iterator iter = route.components.begin();
        iter != route.components.end();
        ++iter){
      components.push_back(*iter);
    }
    nextHop = components.begin();
  }

  StaticRoute::~StaticRoute( ){
    routeLat->delListener(this);
    components.clear();
    DBG_OUT("StaticRoute::~StaticRoute( )" << endl);
  }
}