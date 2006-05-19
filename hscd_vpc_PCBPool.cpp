#include "hscd_vpc_PCBPool.h"

#include "hscd_vpc_datatypes.h"

namespace SystemC_VPC {

  PCBPool::TypePool::InstanceIterator::InstanceIterator(PCBPool::TypePool* pool) : pool(pool) {
  
    this->pool = pool;
    
    if(this->pool->freePCB.size() > 0){
      this->state = PCBPool::TypePool::InstanceIterator::pos_free;
      this->iter = pool->freePCB.begin();
    }else
      if(this->pool->usedPCB.size() > 0){
        this->state = PCBPool::TypePool::InstanceIterator::pos_used;
        this->iter = pool->usedPCB.begin();
    }else
      if(this->pool->lockedPCB.size() > 0){
        this->state = PCBPool::TypePool::InstanceIterator::pos_locked;
        this->iter = this->pool->lockedPCB.begin();
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
          // go to next set of PCB instances
          this->state = PCBPool::TypePool::InstanceIterator::pos_locked;
          this->iter = this->pool->lockedPCB.begin();
        }
      case PCBPool::TypePool::InstanceIterator::pos_locked:
         if(this->iter != this->pool->lockedPCB.end()){
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

  PCBIterator::PCBIterator(std::map<std::string, PCBPool::TypePool* >* pool) {
  
    this->pool = pool;
    this->typeIter = this->pool->begin();
    if(this->typeIter != this->pool->end()){
     this->instanceIter = this->typeIter->second->getInstanceIterator();
     this->typeIter++;
    } 
    
  }

  bool PCBIterator::hasNext(){

    if(this->instanceIter != NULL){
      for(; this->typeIter != this->pool->end() && !this->instanceIter->hasNext(); this->typeIter++){
        delete this->instanceIter;
        this->instanceIter = NULL;
        this->instanceIter = this->typeIter->second->getInstanceIterator();
      }
      return this->instanceIter->hasNext();
    }
    return false;

  }

  ProcessControlBlock const & PCBIterator::getNext(){
  
    return this->instanceIter->getNext();
    
  }
 
  PCBPool::PCBPool() {}

  PCBPool::~PCBPool(){

    std::map<std::string, TypePool* >::iterator iter;
    for(iter = this->typepools.begin(); iter != this->typepools.end(); iter++){
      delete iter->second;
    }

  }
  
  ProcessControlBlock* PCBPool::allocate(std::string type) throw(NotAllocatedException){

    std::map<std::string, TypePool* >::iterator iter;
    iter = this->typepools.find(type);
    if(iter != this->typepools.end() && iter->second != NULL){
      return iter->second->allocate();
    }

    throw NotAllocatedException(type);
  
  }

  int PCBPool::lock(ProcessControlBlock* p) throw(AlreadyLockedException, NotAllocatedException){
    
    std::map<std::string, TypePool* >::iterator iter;
    iter = this->typepools.find(p->getName());
    if(iter != this->typepools.end() && iter->second != NULL){
      return iter->second->lock(p);
    }

    throw NotAllocatedException();
  }
  
  void PCBPool::unlock(std::string type, int lockid) throw(NotLockedException){

    std::map<std::string, TypePool* >::iterator iter;
    iter = this->typepools.find(type);
    if(iter != this->typepools.end() && iter->second != NULL){
      return iter->second->unlock(lockid);
    }

    throw NotLockedException();
  }
      
  void PCBPool::free(ProcessControlBlock* p){

    std::map<std::string, TypePool* >::iterator iter;
    iter = this->typepools.find(p->getName());
    if(iter != this->typepools.end() && iter->second != NULL){
      iter->second->free(p);
    }

  }

  ProcessControlBlock& PCBPool::registerPCB(std::string type){

    std::map<std::string, TypePool*>::iterator iter;
    iter = this->typepools.find(type);
    if(iter == this->typepools.end()){
      TypePool* pool = new TypePool();
      this->typepools[type] = pool;
      return pool->getBase();
    }else{
      return iter->second->getBase();
    }

  }

  bool PCBPool::hasPCBType(std::string type){

    return (this->typepools.count(type) > 0);
  
  }
  
  PCBIterator PCBPool::getPCBIterator(){
    return PCBIterator(&(this->typepools));
  }


  /**
   * SECTION TypePool
   */

  PCBPool::TypePool::~TypePool(){
 
    assert(this->lockedPCB.size() == 0);
    assert(this->usedPCB.size() == 0);

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
    }else{
      instance = new ProcessControlBlock(*(this->base));
      //instance->setPID(PCBPool::pid_count++);
      this->usedPCB[instance->getPID()] = instance;
    }

    return instance;

  }

  int PCBPool::TypePool::lock(ProcessControlBlock* p) throw(AlreadyLockedException, NotAllocatedException){
  
    std::map<int, ProcessControlBlock* >::iterator iter;
    iter = this->usedPCB.find(p->getPID());
    if(iter != this->usedPCB.end()){
      this->usedPCB.erase(iter);
      int lockid = this->lockCount;
      this->lockCount++;
      this->lockedPCB[lockid] = p;
      return lockid;
    }
    //perform error detection
    iter = this->lockedPCB.find(p->getPID());
    if(iter != this->freePCB.end()){
      throw AlreadyLockedException();
    }else{
      throw NotAllocatedException();
    }
    
  }
  
  void PCBPool::TypePool::unlock(int lockid) throw(NotLockedException){
    
    std::map<int, ProcessControlBlock* >::iterator iter;
    iter = this->lockedPCB.find(lockid);
    if(iter != this->lockedPCB.end()){
      this->usedPCB[iter->second->getPID()] = iter->second;
      this->lockedPCB.erase(iter);
    }else{
      throw NotLockedException();
    }

  }
  
  void PCBPool::TypePool::free(ProcessControlBlock* p){

    std::map<int, ProcessControlBlock* >::iterator iter;
    iter = this->usedPCB.find(p->getPID());
    if(iter != this->usedPCB.end()){
      this->usedPCB.erase(iter);
      
      iter = this->freePCB.find(p->getPID());
      if(iter == this->freePCB.end()){
        this->freePCB[p->getPID()] = p;
      }
    }

  }

  PCBPool::TypePool::InstanceIterator* PCBPool::TypePool::getInstanceIterator(){
    return new InstanceIterator(this);
  }
}
