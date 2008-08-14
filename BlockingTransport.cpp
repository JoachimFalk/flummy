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
    for(Components::iterator iter = this->unblockedComponents.begin();
        iter != this->unblockedComponents.end();
        ++iter){

      Task* copy = Director::getInstance().allocateTask(task->getProcessId());
      copy->setFunctionId(task->getFunctionId());
      copy->setTimingScale(task->getTimingScale());
      iter->second = copy;
    }

    assert(!unblockedComponents.empty());
    BlockingTransport * route = new BlockingTransport(*this);
    route->route( EventPair(taskEvents.dii, route) );
  }

  //
  void BlockingTransport::route( EventPair np ){
    if(!unblockedComponents.empty()){
      //EventPair np(pcb->getBlockEvent().dii, pcb->getBlockEvent().latency);
      AbstractComponent* comp;
      Task*              actualTask ;
      if(this->task->isWrite()){
        Components::reference p = unblockedComponents.front();
        comp = p.first;       
        actualTask = p.second;
        unblockedComponents.pop_front();
        blockedComponents.push_front(std::make_pair(comp, actualTask));
      }else{
        Components::reference p = unblockedComponents.back();
        comp = p.first;       
        actualTask = p.second;
        unblockedComponents.pop_back();
        blockedComponents.push_back(std::make_pair(comp, actualTask));
      }
      DBG_OUT("b_transport (" << actualTask->getName()
              << ") on: " << comp->getName() << std::endl);
      actualTask->setBlockEvent(np);

      comp->requestBlockingCompute(actualTask, this);
    } else if(!blockedComponents.empty()){
      DBG_OUT("blocked all resources" << std::endl);
      AbstractComponent * comp = blockedComponents.back().first;
      Task* actualTask = blockedComponents.back().second;
      actualTask->setBlockEvent(np);
      comp->execBlockingCompute(actualTask, this);
      blockedComponents.pop_back();
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
      if(!unblockedComponents.empty()){
        assert(!blockedComponents.empty());
        //AbstractComponent * comp = blockedComponents.back().first;
        Task* actualTask = blockedComponents.back().second;
        if( !actualTask->isAckedBlocking()){
          while(!blockedComponents.empty()){
            Task* task = blockedComponents.back().second;
            AbstractComponent * c = blockedComponents.back().first;
            task->resetBlockingCompute();
            c->abortBlockingCompute(task, this);
            if(this->task->isWrite()){
              unblockedComponents.push_front(blockedComponents.back());
            } else {
              unblockedComponents.push_back(blockedComponents.back());
            }
            blockedComponents.pop_back();
          }
          return;
        }
      }
      route( EventPair(&dummy, this) );
    }
  }

  //
  void BlockingTransport::eventDestroyed(EventWaiter *e){
    DBG_OUT("eventDestroyed" << endl);
  }

  //
  void BlockingTransport::addHop(std::string name, AbstractComponent * hop){
    Task * dummy = NULL;
    unblockedComponents.push_back(std::make_pair(hop, dummy));
  }

  //
  BlockingTransport::BlockingTransport( std::string source, std::string dest ) {
    this->name = "msg_" + source + "_2_" + dest;

    //components.push_back(comp);
    //components.push_back(bus);
  }

  //
  BlockingTransport::BlockingTransport( const BlockingTransport & route ) :
    unblockedComponents(),
    blockedComponents(),
    task(route.task),
    taskEvents(route.taskEvents),
    dummy(),
    name(route.name) {
    this->addListener(this);
    for(Components::const_iterator iter = route.unblockedComponents.begin();
        iter != route.unblockedComponents.end();
        ++iter){
      unblockedComponents.push_back(*iter);
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
