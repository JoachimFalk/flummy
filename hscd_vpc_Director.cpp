/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_Director.cpp
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#include <iostream>

#include <hscd_vpc_Director.h>
#include <hscd_vpc_AbstractComponent.h>
#include <hscd_vpc_Term.h>
#include <hscd_vpc_VPCBuilder.h>

#include <systemc.h>
#include <map>

namespace SystemC_VPC{

  
  std::auto_ptr<Director> Director::singleton(new Director());
 
  /**
   *
   */
   /*
  AbstractComponent& Director::getResource( const char *name ){
    if(!FALLBACKMODE){
      if(1!=mapping_map_by_name.count(name))
  cerr << "Unknown mapping <"<<name<<"> to ??"<<endl; 
      assert(1==mapping_map_by_name.count(name));
      return *(mapping_map_by_name[name]);
    }else{
      return *(mapping_map_by_name["Fallback-Component"]);
    }
  }
  //AbstractComponent& Director::getResource(int process){}
  */
  Director& Director::getResource( const char* name){
    return *(this->singleton);
  }
 
  /**
   *
   */ 
  Director::Director() : end(0){

    VPCBuilder builder((Director*)this);

    builder.buildVPC();

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
        std::cerr << "Director> result_file: "<< this->vpc_result_file << std::endl;
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
    cerr << "~Director()" <<endl;
    
    getReport();
    
  // clear components
    map<string,AbstractComponent*>::iterator it = component_map_by_name.begin();
    
    while(it != component_map_by_name.end()){
    delete it->second;
    it++;
    }
    
    component_map_by_name.clear();
    
    //clear ProcessControlBlocks
    /*
    std::map<std::string, ProcessControlBlock* >::iterator iter;
    for(iter = this->pcb_map_by_name.begin(); iter != this->pcb_map_by_name.end(); iter++){
      delete iter->second;
    }
    
    this->pcb_map_by_name.clear();
    */
  }

  ProcessControlBlock* Director::getProcessControlBlock( const char *name ){
    assert(!FALLBACKMODE);
    return this->pcbPool.allocate(name);
  }

  PCBPool& Director::getPCBPool(){
    return this->pcbPool;
  }

  void Director::compute(const char* name, const char* funcname, VPC_Event* end){
    if(FALLBACKMODE){
#ifdef VPC_DEBUG
      cout << flush;
      cerr << RED("FallBack::compute( ") << WHITE(name) << RED(" , ") << WHITE(funcname) 
	   << RED(" ) at time: " << sc_simulation_time()) << endl;
#endif

      // create Fallback behavior for active and passive mode!
      if( end != NULL ){
	// passive mode: notify end
	end->notify();
      }


      // do nothing, just return
      return;
    }

    
#ifdef VPC_DEBUG
    std::cerr << YELLOW("Director> compute(") << WHITE(name) << YELLOW(",") << WHITE(funcname) << YELLOW(") at: ") << sc_simulation_time() << std::endl;
#endif //VPC_DEBUG
    
    ProcessControlBlock* pcb = this->getProcessControlBlock(name);
    pcb->setFuncName(funcname);
    int lockid = -1;
    
    if( end == NULL ){
      // prepare active mode
      pcb->setBlockEvent(new VPC_Event());
      lockid = this->pcbPool.lock(pcb);
    }else{
      // prepare passiv mode
      pcb->setBlockEvent(end);
    }
    if(1!=mapping_map_by_name.count(name)){
      cerr << "Unknown mapping <"<<name<<"> to ??"<<endl; 
    }
    
    assert(1==mapping_map_by_name.count(name));
    
    
    // get Component
    AbstractComponent* comp = mapping_map_by_name.find(name)->second;
    
#ifdef VPC_DEBUG
    std::cerr << YELLOW("Director> delegating to ") << WHITE(comp->basename()) << std::endl;
#endif //VPC_DEBUG      
    
    // compute task on found component
    assert(!FALLBACKMODE);
    comp->compute(pcb);

    if( end == NULL){
      // active mode -> returns if simulated delay time has expiYELLOW (blocking compute call)
      CoSupport::SystemC::wait(*(pcb->getBlockEvent()));
      delete pcb->getBlockEvent();
      pcb->setBlockEvent(NULL);
      // as psb has been locked -> unlock it
      this->pcbPool.unlock(pcb->getName(), lockid);
      // and free it
      this->pcbPool.free(pcb);
    }
     
  }

  void Director::compute(const char* name, VPC_Event* end){
  
    this->compute(name, "", end);
  
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
    
    this->component_map_by_name.insert(std::pair<std::string, AbstractComponent* >(comp->basename(), comp));
    
  }
    
  /**
   * \brief Implementation of Director::registerMapping
   */
  void Director::registerMapping(const char* taskName, const char* compName){
    assert(!FALLBACKMODE);

    std::map<std::string, AbstractComponent*>::iterator iter;
    iter = this->component_map_by_name.find(compName);
    //check if component is known
    if(iter != this->component_map_by_name.end()){

#ifdef VPC_DEBUG
      std::cout << "Director: registering mapping: "<< taskName << " <-> " << compName << endl;
#endif //VPC_DEBUG
      
      this->mapping_map_by_name[taskName]= iter->second;
      
    }
  
  }
    
  /**
   * \brief Implementation of  Director::generatePCB
   */
  
  ProcessControlBlock& Director::generatePCB(const char* name){
    assert(!FALLBACKMODE);
    
    ProcessControlBlock& pcb = this->pcbPool.registerPCB(name);
    pcb.setName(name);
    return pcb;
  /*  
    iter = this->pcb_map_by_name.find(name);
    if(iter != this->pcb_map_by_name.end()){
      return iter->second;  
    }
      
    ProcessControlBlock* newPCB = new ProcessControlBlock(name);
    
    this->pcb_map_by_name.insert(std::pair<std::string, ProcessControlBlock* >(name, newPCB));
      
    return newPCB;
    */  
  }
/*
  void Director::registerPCB(const char* name, ProcessControlBlock* pcb){

    this->pcbPool.registerPCB(name, pcb);
  
  }
  */  
  /**
   * \brief Implementation of Director::notifyTaskEvent
   */
  void Director::signalTaskEvent(ProcessControlBlock* pcb){
    assert(!FALLBACKMODE);

#ifdef VPC_DEBUG
    std::cerr << "Director> got notified from: " << pcb->getName() << std::endl;
#endif //VPC_DEBUG
    if(pcb->getState() != activation_state(aborted)){
#ifdef VPC_DEBUG
      std::cerr << "Director> task successful finished: " << pcb->getName() << std::endl;
#endif //VPC_DEBUG
      pcb->getBlockEvent()->notify();
      // remember last acknowledged task time
      this->end = sc_simulation_time();
      
      // free allocated pcb
      this->pcbPool.free(pcb);
    }else{
#ifdef VPC_DEBUG
      std::cerr << "Director> re-compute: " << pcb->getName() << std::endl;
#endif //VPC_DEBUG
      // get Component
      AbstractComponent* comp = mapping_map_by_name[pcb->getName()];
      comp->compute(pcb);
    }
    wait(SC_ZERO_TIME);
  } 
  
}

