#include "BlockingTransport.h"
#include "hscd_vpc_Director.h"

#include "debug_config.h"
// if compiled with DBG_STATIC_ROUTE create stream and include debug macros
#ifdef DBG_BLOCKING_TRANSPORT
#include <CoSupport/Streams/DebugOStream.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include "debug_on.h"
#else
  #include "debug_off.h"
#endif

namespace SystemC_VPC {

  //
  void BlockingTransport::compute( Task* _task ) {
    task = _task;
    taskEvents = task->getBlockEvent();
    assert(!components.empty());
    BlockingTransport * route = new BlockingTransport(*this);
    route->route( EventPair(taskEvents.dii, route) );
  }

  //
  void BlockingTransport::route( EventPair np ){
    if(!components.empty()){
      //EventPair np(pcb->getBlockEvent().dii, pcb->getBlockEvent().latency);
      Task *newTask =
        Director::getInstance().allocateTask(task->getProcessId());
      newTask->setBlockEvent(np);
      newTask->setFunctionId(task->getFunctionId());
      DBG_OUT("route on: " << components.front()->getName() << endl);
      components.front()->compute(newTask);
      components.pop_front();
    } else {
      Director::getInstance().signalProcessEvent(task);
      //taskEvents.latency->notify();
    }
  }

  //
  void BlockingTransport::signaled(EventWaiter *e) {
    if(e->isActive()){
      DBG_OUT("signaled @ " << sc_time_stamp() << endl);
      this->reset();
      route( EventPair(&dummy, this) );
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
  BlockingTransport::BlockingTransport( std::string source, std::string dest ) {
    this->name = "msg_" + source + "_2_" + dest;

    //components.push_back(comp);
    //components.push_back(bus);
  }

  //
  BlockingTransport::BlockingTransport( const BlockingTransport & route ) :
    components(),
    task(route.task),
    taskEvents(route.taskEvents),
    dummy(),
    name(route.name) {
    this->addListener(this);
    for(Components::const_iterator iter = route.components.begin();
        iter != route.components.end();
        ++iter){
      components.push_back(*iter);
    }
  }

  BlockingTransport::~BlockingTransport( ){
    DBG_OUT("BlockingTransport::~BlockingTransport( )" << endl);
  }

  //
  const char* BlockingTransport::getName() const {
    return this->name.c_str();
  }
}
