// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#include "BlockingTransport.hpp"
#include "../DebugOStream.hpp"

namespace SystemC_VPC { namespace Detail { namespace Routing {

  const char *BlockingTransport::Type = "BlockingTransport";

  //
  BlockingTransport::BlockingTransport(std::string const &name)
    : AbstractRoute(name,
        reinterpret_cast<char *>(static_cast<Route         *>(this)) -
        reinterpret_cast<char *>(static_cast<AbstractRoute *>(this)))
    , Route(Type,
        reinterpret_cast<char *>(static_cast<AbstractRoute *>(this)) -
        reinterpret_cast<char *>(static_cast<Route         *>(this)))
    , isWrite(false)
    , task(nullptr)
    , dummyDii(new VPCEvent())
    , routeLat(new VPCEvent())
    , phase(LOCK_ROUTE)
  {
//  routeLat->addListener(this);
    //components.push_back(comp);
    //components.push_back(bus);
  }

  void BlockingTransport::start(size_t quantitiy, void *userData, CallBack completed) {
    (*completed)(userData, quantitiy, nullptr);
  }

  void BlockingTransport::finalize() {
  }

//void BlockingTransport::compute( TaskInstanceImpl* _task ) {
//  task = _task;
//  this->isWrite = task->isWrite();
//  //this->reset();
//
//  if(hopList.empty()){
//    if(components.empty()){
//      // this route is empty -> return immediately
//      Director::getInstance().signalLatencyEvent(task);
//      DBG_OUT("EMPTY" << std::endl);
////      this->pool->free(this);
//      return;
//    }
//
//    // enter this, on the first call of compute:
//    // fill the route list
//    taskEvents = task->getBlockEvent();
//    for(ComponentList::iterator iter = this->components.begin();
//      iter != this->components.end();
//      ++iter){
//      TaskInstanceImpl* copy =
//        Director::getInstance().allocateTask(task->getProcessId());
//      copy->setFunctionIds(task->getFunctionIds());
//      copy->setTimingScale(task->getTimingScale());
//      copy->setTask((*iter)->getTask(task->getProcessId()));
//      hopList.push_back( std::make_pair(*iter, copy) );
//      // we need to lock the route in write or in read direction
//      if(this->isWrite){
//        // write: in order of the route list
//        lockList.push_back( std::make_pair(*iter, copy) );
//      }else{
//        // read: opposite order
//        lockList.push_front( std::make_pair(*iter, copy) );
//      }
//    }
//    //phase   = LOCK_ROUTE;
//    //nextHop = lockList.begin();
//    this->resetLists();
//  }
//
//  //assert(!unblockedComponents.empty());
//  //BlockingTransport * route = new BlockingTransport(*this);
//  //route->route( EventPair(taskEvents.dii, route) );
//  this->route( EventPair(taskEvents.dii, routeLat) );
//}
//
////
//void BlockingTransport::route( EventPair np ){
//  DBG_OUT("b_transport"
//          << " (" << this->getName() << ") "
//          << std::endl);
//
//  DBG_OUT( " "
//           << (phase == LOCK_ROUTE) << ", "
//           << (nextHop != lockList.end()) << ", "
//           << (phase == COMPUTE_ROUTE) << ", "
//           << (nextHop != hopList.end())  << std::endl);
//
//  if( phase == LOCK_ROUTE && nextHop != lockList.end() ){
//    //EventPair np(taskImpl->getBlockEvent().dii, taskImpl->getBlockEvent().latency);
//    AbstractComponent* comp       = nextHop->first;
//    TaskInstanceImpl*              actualTask = nextHop->second;
//    actualTask->setBlockEvent(np);
//    DBG_OUT("lock " << this->getName()
//            << "on: " << comp->getName()
//            << std::endl);
//
//    // count iter for next hop if blocking compute is akk`ed only
//    /*
//    ++nextHop;
//    if(nextHop == lockList.end()){
//      phase=COMPUTE_ROUTE;
//      nextHop = hopList.begin();
//    }
//    */
//
//    comp->requestBlockingCompute(actualTask, routeLat);
//  } else if( phase == COMPUTE_ROUTE && nextHop != hopList.end() ){
//    AbstractComponent * comp = nextHop->first;
//    TaskInstanceImpl* actualTask = nextHop->second;
//    DBG_OUT("compute " << this->getName()
//            << "on: " << comp->getName()
//            << std::endl);
//
//    actualTask->setBlockEvent(np);
//    comp->execBlockingCompute(actualTask, routeLat);
//
//    ++nextHop;
//  } else {
//    assert( nextHop == hopList.end() );
//    Director::getInstance().signalLatencyEvent(task);
//    this->resetLists();
////    this->pool->free(this);
//    return; // paranoia
//  }
//}
//
////
//void BlockingTransport::signaled(EventWaiter *e) {
//  if(e->isActive()){
//    DBG_OUT("signaled"
//            << " (" << this->getName() << ") "
//            << " @ " << sc_core::sc_time_stamp() << std::endl);
//
//    DBG_OUT( " "
//             << (phase == LOCK_ROUTE) << ", "
//             << (nextHop != lockList.end()) << ", "
//             << (phase == COMPUTE_ROUTE) << ", "
//             << (nextHop != hopList.end())  << std::endl);
//
//    routeLat->reset();
//
//    if( phase == LOCK_ROUTE && nextHop != lockList.end() ){
//      assert(!hopList.empty());
//      //AbstractComponent * comp = blockedComponents.back().first;
//      TaskInstanceImpl* actualTask = nextHop->second;
//      // we need to test if a hop is locked already
//      if( actualTask->isAckedBlocking()){
//        // count iter for next hop if blocking compute is akk`ed only
//        ++nextHop;
//        if(nextHop == lockList.end()){
//          phase=COMPUTE_ROUTE;
//          nextHop = hopList.begin();
//        }
//
//      }else{
//        // the hop is locked -> reset route
//        DBG_OUT("  is NOT blocking: "
//                << " (" << this->getName() << ") "
//                << " @ " << sc_core::sc_time_stamp() << std::endl);
//
//        // in this case we have to release locks
//        // and we need to restart locking
//        this->resetHops();
//        this->resetLists();
//        return;
//      }
//    }
//    route( EventPair(dummyDii, routeLat) );
//  }
//}
//
////
//void BlockingTransport::eventDestroyed(EventWaiter *e){
//  DBG_OUT("eventDestroyed" << std::endl);
//}

  //
  void BlockingTransport::addHop(std::string name, AbstractComponent * hop){
    components.push_back(hop);
  }

  //
  const BlockingTransport::ComponentList& BlockingTransport::getHops() const {
    return components;
  }

  //
  void BlockingTransport::resetHops(){
    DBG_OUT("resetHops()"
            << " (" << this->getName() << ") "
            << " @ " << sc_core::sc_time_stamp() << std::endl);
    for(Components::iterator iter = lockList.begin();
        iter != lockList.end();
        ++iter){
      TaskInstanceImpl* task = iter->second;
      AbstractComponent * c = iter->first;
      task->resetBlockingCompute();
      c->abortBlockingCompute(task, routeLat);
    }
  }

  //
  void BlockingTransport::resetLists(){
    DBG_OUT("resetLists()"
            << " (" << this->getName() << ") "
            << " @ " << sc_core::sc_time_stamp() << std::endl);
    phase   = LOCK_ROUTE;
    nextHop = lockList.begin();
  }

  BlockingTransport::~BlockingTransport( ){
//  routeLat->delListener(this);
    DBG_OUT("BlockingTransport::~BlockingTransport( )" << std::endl);
  }

} } } // namespace SystemC_VPC::Detail::Routing
