/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
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
