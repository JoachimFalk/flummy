#include "StaticRoute.h"

namespace SystemC_VPC {

  //
  void StaticRoute::compute( ProcessControlBlock *pcb ) {
    task = pcb;
    taskEvents = task->getBlockEvent();
    assert(!components.empty());
    StaticRoute * route = new StaticRoute(*this);
    route->route( EventPair(taskEvents.dii, route) );
  }

  //
  void StaticRoute::route( EventPair np ){
    if(!components.empty()){
      //EventPair np(pcb->getBlockEvent().dii, pcb->getBlockEvent().latency);
      task->setBlockEvent(np);
      cerr << "route on: " << components.front()->getName() << endl;
      components.front()->compute(task);
      components.pop_front();
    } else {
      taskEvents.latency->notify();
    }
  }

  //
  void StaticRoute::signaled(EventWaiter *e) {
    if(e->isActive()){
      cerr << "signaled @ " << sc_time_stamp() << endl;
      this->reset();
      route( EventPair(&dummy, this) );
    }
  }

  //
  void StaticRoute::eventDestroyed(EventWaiter *e){
    cerr << "eventDestroyed" << endl;
  }

  //
  void StaticRoute::addHop(std::string name, AbstractComponent * hop){
    components.push_back(hop);
  }

  //
  StaticRoute::StaticRoute( std::string source, std::string dest ) {
    this->name = "msg_" + source + "_2_" + dest;

    //components.push_back(comp);
    //components.push_back(bus);
  }

  //
  StaticRoute::StaticRoute( const StaticRoute & route ) :
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

  StaticRoute::~StaticRoute( ){
    cerr << "StaticRoute::~StaticRoute( )" << endl;
  }

  //
  const char* StaticRoute::getName() const {
    return this->name.c_str();
  }
}
