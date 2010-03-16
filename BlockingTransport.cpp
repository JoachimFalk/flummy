#include <systemcvpc/BlockingTransport.h>
#include <systemcvpc/RoutePool.h>
#include <systemcvpc/hscd_vpc_Director.h>

#include <systemcvpc/debug_config.h>
// if compiled with DBG_STATIC_ROUTE create stream and include debug macros
#ifdef DBG_BLOCKING_TRANSPORT
#include <CoSupport/Streams/DebugOStream.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include <systemcvpc/debug_on.h>
#else
  #include <systemcvpc/debug_off.h>
#endif

namespace SystemC_VPC {

  //
  void BlockingTransport::compute( Task* _task ) {
    task = _task;
    this->isWrite = task->isWrite();
    //this->reset();

    if(hopList.empty()){
      if(components.empty()){
        // this route is empty -> return immediately
        Director::getInstance().signalProcessEvent(task);
        DBG_OUT("EMPTY" << std::endl);
        this->pool->free(this);
        return;
      }

      // enter this, on the first call of compute:
      // fill the route list
      taskEvents = task->getBlockEvent();
      for(ComponentList::iterator iter = this->components.begin();
        iter != this->components.end();
        ++iter){
        Task* copy =
          Director::getInstance().allocateTask(task->getProcessId());
        copy->setFunctionId(task->getFunctionId());
        copy->setTimingScale(task->getTimingScale());
        hopList.push_back( std::make_pair(*iter, copy) );
        // we need to lock the route in write or in read direction
        if(this->isWrite){
          // write: in order of the route list
          lockList.push_back( std::make_pair(*iter, copy) );
        }else{
          // read: opposite order
          lockList.push_front( std::make_pair(*iter, copy) );
        }
      }
      //phase   = LOCK_ROUTE;
      //nextHop = lockList.begin();
      this->resetLists();
    }

    //assert(!unblockedComponents.empty());
    //BlockingTransport * route = new BlockingTransport(*this);
    //route->route( EventPair(taskEvents.dii, route) );
    this->route( EventPair(taskEvents.dii, routeLat) );
  }

  //
  void BlockingTransport::route( EventPair np ){
    DBG_OUT("b_transport"
            << " (" << this->getName() << ") "
            << std::endl);

    DBG_OUT( " "
             << (phase == LOCK_ROUTE) << ", "
             << (nextHop != lockList.end()) << ", "
             << (phase == COMPUTE_ROUTE) << ", "
             << (nextHop != hopList.end())  << std::endl);

    if( phase == LOCK_ROUTE && nextHop != lockList.end() ){
      //EventPair np(pcb->getBlockEvent().dii, pcb->getBlockEvent().latency);
      AbstractComponent* comp       = nextHop->first;
      Task*              actualTask = nextHop->second;
      actualTask->setBlockEvent(np);
      DBG_OUT("lock " << this->getName()
              << "on: " << comp->getName()
              << std::endl);

      // count iter for next hop if blocking compute is akk`ed only
      /*
      ++nextHop;
      if(nextHop == lockList.end()){
        phase=COMPUTE_ROUTE;
        nextHop = hopList.begin();
      }
      */

      comp->requestBlockingCompute(actualTask, routeLat);
    } else if( phase == COMPUTE_ROUTE && nextHop != hopList.end() ){
      AbstractComponent * comp = nextHop->first;
      Task* actualTask = nextHop->second;
      DBG_OUT("compute " << this->getName()
              << "on: " << comp->getName()
              << std::endl);

      actualTask->setBlockEvent(np);
      comp->execBlockingCompute(actualTask, routeLat);

      ++nextHop;
    } else {
      assert( nextHop == hopList.end() );
      Director::getInstance().signalProcessEvent(task);
      this->resetLists();
      this->pool->free(this);
      return; // paranoia
    }
  }

  //
  void BlockingTransport::signaled(EventWaiter *e) {
    if(e->isActive()){
      DBG_OUT("signaled"
              << " (" << this->getName() << ") "
              << " @ " << sc_time_stamp() << endl);

      DBG_OUT( " "
               << (phase == LOCK_ROUTE) << ", "
               << (nextHop != lockList.end()) << ", "
               << (phase == COMPUTE_ROUTE) << ", "
               << (nextHop != hopList.end())  << std::endl);

      routeLat->reset();      

      if( phase == LOCK_ROUTE && nextHop != lockList.end() ){
        assert(!hopList.empty());
        //AbstractComponent * comp = blockedComponents.back().first;
        Task* actualTask = nextHop->second;
        // we need to test if a hop is locked already
        if( actualTask->isAckedBlocking()){
          // count iter for next hop if blocking compute is akk`ed only
          ++nextHop;
          if(nextHop == lockList.end()){
            phase=COMPUTE_ROUTE;
            nextHop = hopList.begin();
          }

        }else{
          // the hop is locked -> reset route
          DBG_OUT("  is NOT blocking: "
                  << " (" << this->getName() << ") "
                  << " @ " << sc_time_stamp() << endl);
          
          // in this case we have to release locks
          // and we need to restart locking
          this->resetHops();
          this->resetLists();
          return;
        }
      }
      route( EventPair(dummyDii, routeLat) );
    }
  }

  //
  void BlockingTransport::eventDestroyed(EventWaiter *e){
    DBG_OUT("eventDestroyed" << endl);
  }

  //
  void BlockingTransport::addHop(std::string name, AbstractComponent * hop){
    components.push_back(hop);
  }

  //
  void BlockingTransport::setPool(RoutePool<BlockingTransport> * pool)
  {
    this->pool = pool;
  }

  //
  const ComponentList& BlockingTransport::getHops() const {
    return components;
  }

  //
  BlockingTransport::BlockingTransport( std::string source, std::string dest )
    : components(),
      dummyDii(new CoSupport::SystemC::RefCountEvent()),
      routeLat(new CoSupport::SystemC::RefCountEvent()),
      phase(LOCK_ROUTE)
  {
    this->name = "msg_" + source + "_2_" + dest;
    routeLat->addListener(this);

    //components.push_back(comp);
    //components.push_back(bus);
  }

  //
  BlockingTransport::BlockingTransport( const BlockingTransport & route ) :
    components(),
    task(route.task),
    taskEvents(route.taskEvents),
    dummyDii(new CoSupport::SystemC::RefCountEvent()),
    routeLat(new CoSupport::SystemC::RefCountEvent()),
    name(route.name) {
    DBG_OUT("copy a BlockingTransport orig=" << &route << std::endl);
    routeLat->addListener(this);
    for(ComponentList::const_iterator iter = route.components.begin();
        iter != route.components.end();
        ++iter){
      components.push_back(*iter);
    }
  }

  BlockingTransport::~BlockingTransport( ){
    routeLat->delListener(this);
    DBG_OUT("BlockingTransport::~BlockingTransport( )" << endl);
  }

  //
  const char* BlockingTransport::getName() const {
    return this->name.c_str();
  }

  //
  void BlockingTransport::resetHops(){
    DBG_OUT("resetHops()"
            << " (" << this->getName() << ") "
            << " @ " << sc_time_stamp() << endl);
    for(Components::iterator iter = lockList.begin();
        iter != lockList.end();
        ++iter){
      Task* task = iter->second;
      AbstractComponent * c = iter->first;
      task->resetBlockingCompute();
      c->abortBlockingCompute(task, routeLat);
    }
  }

  //
  void BlockingTransport::resetLists(){
    DBG_OUT("resetLists()"
            << " (" << this->getName() << ") "
            << " @ " << sc_time_stamp() << endl);
    phase   = LOCK_ROUTE;
    nextHop = lockList.begin();
  }
}
