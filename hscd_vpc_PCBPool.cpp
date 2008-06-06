#include "hscd_vpc_PCBPool.h"

namespace SystemC_VPC {

  PCBPool::TypePool::InstanceIterator::
    InstanceIterator(PCBPool::TypePool* pool) : pool(pool) {
  
    this->pool = pool;
    
    if(this->pool->freePCB.size() > 0){
      this->state = PCBPool::TypePool::InstanceIterator::pos_free;
      this->iter = pool->freePCB.begin();
    }else
      if(this->pool->usedPCB.size() > 0){
        this->state = PCBPool::TypePool::InstanceIterator::pos_used;
        this->iter = pool->usedPCB.begin();
    }else{
      this->state = PCBPool::TypePool::InstanceIterator::pos_end;
    }
    
  }
  
  bool PCBPool::TypePool::InstanceIterator::hasNext() {
    
    switch(this->state){
      case PCBPool::TypePool::InstanceIterator::pos_free:
        if(this->iter != this->pool->freePCB.end()){
          return true;
        }else{
          // go to next set of PCB instances
          this->state = PCBPool::TypePool::InstanceIterator::pos_used;
          this->iter = this->pool->usedPCB.begin();
        }
      case PCBPool::TypePool::InstanceIterator::pos_used:
         if(this->iter != this->pool->usedPCB.end()){
          return true;
        }else{
           // we reached the end so mark it!
           this->state = PCBPool::TypePool::InstanceIterator::pos_end;
         }
      case PCBPool::TypePool::InstanceIterator::pos_end:
        return false;
    }
    
    return false;
  }
  
  ProcessControlBlock const & PCBPool::TypePool::InstanceIterator::getNext() {
    
    ProcessControlBlock* pcb = this->iter->second;
    this->iter++;
    return *pcb;

  }

  PCBIterator::PCBIterator(PCBPool::TypePools * pool) {
  
    this->pool = pool;
    this->typeIter = this->pool->begin();
    if(this->typeIter != this->pool->end()){
     this->instanceIter = (*this->typeIter)->getInstanceIterator();
     this->typeIter++;
    } 
    
  }

  bool PCBIterator::hasNext(){

    if(this->instanceIter != NULL){
      for(; this->typeIter != this->pool->end()
            && !this->instanceIter->hasNext();
          ++(this->typeIter) ){
        delete this->instanceIter;
        this->instanceIter = NULL;
        this->instanceIter = (*this->typeIter)->getInstanceIterator();
      }
      return this->instanceIter->hasNext();
    }
    return false;

  }

  ProcessControlBlock const & PCBIterator::getNext(){
  
    return this->instanceIter->getNext();
    
  }
 
  PCBPool::PCBPool() : typepools(0) {}

  PCBPool::~PCBPool(){

    TypePools::iterator iter;
    for(iter = this->typepools.begin(); iter != this->typepools.end(); iter++){
      delete *iter;
    }

  }
  
  ProcessControlBlock* PCBPool::allocate( ProcessId pid )
    throw(NotAllocatedException){

    TypePool *pool = typepools[pid];
    if( pool != NULL ){
      return pool->allocate();
    }

    throw NotAllocatedException(pid);
  
  }

  void PCBPool::free(ProcessControlBlock* p){

    TypePool *pool = typepools[p->getPid()];
    if( pool != NULL ){
       pool->free(p);
    }

  }

  ProcessControlBlock& PCBPool::registerPCB(ProcessId pid){

    if( pid == typepools.size() ){
      typepools.push_back( new TypePool());
    }

    return this->typepools[pid]->getBase();
  }

  PCBIterator PCBPool::getPCBIterator(){
    return PCBIterator(&(this->typepools));
  }


  /**
   * SECTION TypePool
   */

  PCBPool::TypePool::~TypePool(){

    if(this->usedPCB.size() != 0){
      std::cerr << "WARNING: TypePool for " << this->base->getName()
                << " still used instances exist!" << std::endl
                << "Assuming interruption by sc_stop happend."
        " Cleaning up all instances of " << this->base->getName() << std::endl;
    }

    std::map<int, ProcessControlBlock* >::iterator iter;
    for(iter = this->freePCB.begin(); iter != this->freePCB.end(); iter++){
      delete iter->second;
    }

  }
  
  ProcessControlBlock* PCBPool::TypePool::allocate(){

    ProcessControlBlock* instance = NULL;

    if(this->freePCB.size() > 0){
      std::map<int, ProcessControlBlock* >::iterator iter;
      iter = this->freePCB.begin();
      instance = iter->second;
      this->usedPCB[iter->first] = instance;
      this->freePCB.erase(iter);
    }else{
      instance = new ProcessControlBlock(*(this->base));
      this->usedPCB[instance->getInstanceId()] = instance;
    }

    return instance;

  }

  void PCBPool::TypePool::free(ProcessControlBlock* p){

    std::map<int, ProcessControlBlock* >::iterator iter;
    iter = this->usedPCB.find(p->getInstanceId());
    if(iter != this->usedPCB.end()){
      this->usedPCB.erase(iter);
      
      iter = this->freePCB.find(p->getInstanceId());
      if(iter == this->freePCB.end()){
        this->freePCB[p->getInstanceId()] = p;
      }
    }
  
  }

  PCBPool::TypePool::InstanceIterator* PCBPool::TypePool::getInstanceIterator()
  {
    return new InstanceIterator(this);
  }
}
