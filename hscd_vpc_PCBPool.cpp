#include "hscd_vpc_PCBPool.h"

namespace SystemC_VPC {

  ProcessControlBlock* PCBPool::allocate(std::string type) throw(NotAllocatedException){

    std::map<std::string, TypePool* >::iterator iter;
    iter = this->typepools.find(type);
    if(iter != this->typepools.end() && iter->second != NULL){
      return iter->second->allocate();
    }

    throw NotAllocatedException();
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
      iter->second->getBase();
    }

  }

  bool PCBPool::hasPCBType(std::string type){

    return (this->typepools.count(type) > 0);
  
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

}
