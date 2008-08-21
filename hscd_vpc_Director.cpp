/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * hscd_vpc_Director.cpp
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/
#include <iostream>

#include <hscd_vpc_Director.h>
#include <hscd_vpc_AbstractComponent.h>
#include <hscd_vpc_Term.h>
#include <hscd_vpc_VPCBuilder.h>
#include <hscd_vpc_OnlineBinder.h>

#include <systemc.h>
#include <map>

#include "hscd_vpc_AbstractBinder.h"
#include "hscd_vpc_PriorityBinder.h"
#include "hscd_vpc_LeastCurrentlyBoundPE.h"

namespace SystemC_VPC{

  //
  std::auto_ptr<Director> Director::singleton(new Director());

  //
  Director& Director::getResource( const char* name){
    return *(this->singleton);
  }

  /**
   *
   */
  Director::Director() : end(0), binder(NULL),reconfigurationBlockedUntil(SC_ZERO_TIME), FALLBACKMODE(false) {

    try{
      VPCBuilder builder((Director*)this);

      // set default binder
      PriorityElementFactory* factory = new LCBPEFactory();
      this->binder = new PriorityBinder(factory);

      builder.buildVPC();
    }catch(InvalidArgumentException& e){
      std::cerr << "Director> Got exception while setting up VPC:\n"
                << e.what() << std::endl;
      exit(-1);
    }catch(const std::exception& e){
      std::cerr << "Director> Got exception while setting up VPC:\n"
                << e.what() << std::endl;
      exit(-1);
    }

  }

  /**
   *
   */
  void Director::checkConstraints() {
    vector<Constraint*>::const_iterator iter=constraints.begin();
    for(;iter!=constraints.end();iter++){
      (*iter)->isSatisfied();
    }
  }

  /**
   *
   */
  void Director::getReport(){
    vector<Constraint*>::const_iterator iter=constraints.begin();
    char *cons_name;
    double start=-1;
    double end=-1;
    // if there are any constaints to be viewed calculate time
    if(this->constraints.size() > 0){
      for(;iter!=constraints.end();iter++){
        cons_name=(*iter)->getName();
        if(0==strncmp("start",cons_name,5))
          start=(*iter)->getSatisfiedTime();
        else if(0==strncmp("end",cons_name,3))
          end=(*iter)->getSatisfiedTime();
      }
    }else{ // else take total simulation time
      start = 0;
      end = this->end;
    }
#ifdef VPC_DEBUG
    std::cerr << "start: " << start << " end: " << end  << std::endl;
#endif //VPC_DEBUG
    if ((start != -1) && (end != -1)){
      if(0 != this->vpc_result_file.compare("")){

#ifdef VPC_DEBUG
        std::cerr << "Director> result_file: "
                  << this->vpc_result_file << std::endl;
#endif //VPC_DEBUG
        ofstream resultFile;
        resultFile.open(this->vpc_result_file.c_str());
        if(resultFile){
          resultFile << (end-start);
        }
        resultFile.flush();
        resultFile.close();
      }else{
        std::cerr << "latency: " << end - start << std::endl;
      }
    }
  }

  /**
   *
   */
  Director::~Director(){
    //cerr << "~Director()" <<endl;

    getReport();

  // clear components
    map<string,AbstractComponent*>::iterator it =
      component_map_by_name.begin();

    while(it != component_map_by_name.end()){
      delete it->second;

      it++;
    }

    component_map_by_name.clear();

    delete this->binder;

  }

  ProcessControlBlock* Director::getProcessControlBlock( const char *name ){
    assert(!FALLBACKMODE);

    try{
      return this->pcbPool.allocate(name);
    }catch(NotAllocatedException& e){
      std::cerr << "Director> getProcessControlBlock failed due to" << std::endl
		<< e.what() << std::endl;
      std::cerr << "HINT: probably actor binding not specified in configuration file!" << std::endl;
      exit(-1);
    }
  }

  PCBPool& Director::getPCBPool(){
    return this->pcbPool;
  }

  void Director::setBinder(AbstractBinder* b){
    if(b != NULL){
      //free old binder
      if(this->binder != NULL){
        delete this->binder;
      }
      this->binder = b;
      //b->setDirector(this); 
    }
  }

  AbstractBinder* Director::getBinder(){
    return this->binder;
  }

  void Director::compute(const char* name, const char* funcname, VPC_Event* end){
    //HINT: treat mode!!
    //if (mode) { ....
    compute(name, funcname, EventPair(end, NULL));
    //} else{
    //  compute(name, funcname, EventPair(NULL, end));
    //}
  }

  void Director::compute(const char* name, const char* funcname, EventPair endPair){
    if(FALLBACKMODE){
#ifdef VPC_DEBUG
      cout << flush;
      cerr << VPC_RED("FallBack::compute( ") << VPC_WHITE(name)
           << VPC_RED(" , ") << VPC_WHITE(funcname)
	   << VPC_RED(" ) at time: " << sc_simulation_time()) << endl;
#endif

      // create Fallback behavior for active and passive mode!
      if( endPair.dii != NULL )
        endPair.dii->notify();      // passive mode: notify end
      if( endPair.latency != NULL )
        endPair.latency->notify();  // passive mode: notify end

      // do nothing, just return
      return;
    }

#ifdef VPC_DEBUG
    std::cerr << VPC_YELLOW("Director> compute(") << VPC_WHITE(name)
              << VPC_YELLOW(",") << VPC_WHITE(funcname)
              << VPC_YELLOW(") at: ") << sc_simulation_time() << std::endl;
#endif //VPC_DEBUG

    ProcessControlBlock* pcb = this->getProcessControlBlock(name);
    pcb->setFuncName(funcname);
    int lockid = -1;

    //HINT: also treat mode!!
    //if( endPair.latency != NULL ) endPair.latency->notify();

    if( endPair.dii == NULL ){
      // prepare active mode
      pcb->setBlockEvent(EventPair(new VPC_Event(), new VPC_Event()));
      lockid = this->pcbPool.lock(pcb);
    }else{
      // prepare passiv mode
      pcb->setBlockEvent(endPair);
    }

    try{
      // get Component
      std::string compName = this->binder->resolveBinding(*pcb, NULL);
      AbstractComponent* comp =
        this->component_map_by_name.find(compName)->second;

#ifdef VPC_DEBUG
      std::cerr << VPC_YELLOW("Director> delegating to ")
                << VPC_WHITE(comp->basename()) << std::endl;
#endif //VPC_DEBUG

      // compute task on found component
      assert(!FALLBACKMODE);
      comp->compute(pcb);

    }catch(UnknownBindingException& e){
      std::cerr << e.what() << std::endl;

      // clean up
      if( endPair.dii == NULL ){
        delete pcb->getBlockEvent().dii;
        delete pcb->getBlockEvent().latency;

        pcb->setBlockEvent(EventPair());
        this->pcbPool.unlock(pcb->getName(), lockid);
        this->pcbPool.free(pcb);
      }else{
        // TODO: handle asynchron calls notify them ?!?
      }
      return;

    }

    if( endPair.dii == NULL){
      // active mode -> returns if simulated delay time has expired
      // (blocking compute call)
      CoSupport::SystemC::wait(*(pcb->getBlockEvent().dii));
      delete pcb->getBlockEvent().dii;
      delete pcb->getBlockEvent().latency;
      pcb->setBlockEvent(EventPair());

      // as psb has been locked -> unlock it
      this->pcbPool.unlock(pcb->getName(), lockid);
      // and free it
      this->pcbPool.free(pcb);
    }

  }

  void Director::compute(const char *name, EventPair endPair){
    compute(name, "", endPair);
  }

  void Director::compute(const char* name, VPC_Event* end){

    compute(name, "", EventPair(end, NULL));

  }

  /**
   * \brief Implementation of Director::addConstraint
   */
  void Director::addConstraint(Constraint* cons){

    this->constraints.push_back(cons);

  }

  /**
   * \brief Implementation of Director::registerComponent
   */
  void Director::registerComponent(AbstractComponent* comp){

    this->component_map_by_name.insert(
      std::pair<std::string, AbstractComponent* >(comp->basename(), comp));

  }
  
  /**
   * \brief Implementation of  Director::generatePCB
   */

  ProcessControlBlock& Director::generatePCB(const char* name){
    assert(!FALLBACKMODE);

    ProcessControlBlock& pcb = this->pcbPool.registerPCB(name);
    pcb.setName(name);
    return pcb;
  }

  /**
   * \brief Implementation of Director::notifyTaskEvent
   */
  void Director::signalProcessEvent(ProcessControlBlock* pcb,
                                    std::string compID){
    assert(!FALLBACKMODE);

    // inform binder about task event
    this->binder->signalProcessEvent(pcb, compID);

#ifdef VPC_DEBUG
    std::cerr << "Director> got notified from: " << pcb->getName() << ":"
              << pcb->getFuncName() << " at " << sc_simulation_time()
              << std::endl;
#endif //VPC_DEBUG
    if(pcb->getState() != activation_state(aborted)){
#ifdef VPC_DEBUG
      std::cerr << "Director> task successful finished: " << pcb->getName()
                << std::endl;
#endif //VPC_DEBUG
      if(NULL != pcb->getBlockEvent().latency)
        pcb->getBlockEvent().latency->notify();
      // remember last acknowledged task time
      this->end = sc_simulation_time();

      // free allocated pcb
      this->pcbPool.free(pcb);
    }else{
#ifdef VPC_DEBUG
      std::cerr << "Director> re-compute: " << pcb->getName() << std::endl;
#endif //VPC_DEBUG
      // resolve binding again
      try{
        // get Component
        std::string compName = this->binder->resolveBinding(*pcb, NULL);
        AbstractComponent* comp =
          this->component_map_by_name.find(compName)->second;

#ifdef VPC_DEBUG
        std::cerr << VPC_YELLOW("Director> re-delegating to ")
                  << VPC_WHITE(comp->basename()) << std::endl;
#endif //VPC_DEBUG

        // compute task on found component
        assert(!FALLBACKMODE);
        comp->compute(pcb);

      }catch(UnknownBindingException& e){
        std::cerr << "Director> re-computer failed: " << e.what() << std::endl;
        return;
      }
    }
    wait(SC_ZERO_TIME);
  }
   
   /**
   * \brief Implementation of Director::getCompByName
   * \return ReconfigurableComponent* generated by Director from Comp string
   */
  ReconfigurableComponent* Director::getCompByName(std::string Comp){
    
    ReconfigurableComponent* myComp = dynamic_cast<ReconfigurableComponent*>(this->component_map_by_name.find(Comp)->second);
    
    return myComp;
  }

  /**
   * \brief Implementation of Director::getReconfigurationWaitInterval
   * \return sc_time delay generated by other reconfiguration
   */
  sc_time Director::getReconfigurationWaitInterval(sc_time newSetupTime, ReconfigurableComponent* ReComp){
    /*if(this->reconfigurationBlockedUntil <= sc_time_stamp()){
      this->reconfigurationBlockedUntil = sc_time_stamp() + newSetupTime;
      return SC_ZERO_TIME;
    }else{
      sc_time waitIntervall = this->reconfigurationBlockedUntil - sc_time_stamp();
      this->reconfigurationBlockedUntil += newSetupTime;
      OnlineBinder* myBinder = dynamic_cast<OnlineBinder*>(this->binder);
      myBinder->updateTimesTable(ReComp->basename(), waitIntervall);
      return waitIntervall;
    }*/
    return SC_ZERO_TIME;
  }  
}
