#include "hscd_vpc_Allocator.h"

namespace SystemC_VPC{
  
  Allocator::Allocator(AbstractController* controller) 
    : controller(controller), 
      kill(false), 
      waitInterval(NULL) {}
 
  Allocator::~Allocator() {}

  /**
   * \brief Implementation of  Allocator::setProperty
   */
  bool Allocator::setProperty(char* key, char* value){

    bool used = false;

    // check if property relevant for allocator
    if(0 != std::strncmp(key, ALLOCATORPREFIX, strlen(ALLOCATORPREFIX))){
      return used;
    }else{
      key += strlen(ALLOCATORPREFIX);
    }
   
    if(0 == strcmp(key, "mode")){

#ifdef VPC_DEBUG
      std::cerr << VPC_BLUE("Allocator> Found input data for preemption mode = ") << value << std::endl;
#endif //VPC_DEBUG
      if(0 == strcmp(value, "kill")){
        this->setPreemptionStrategy(true);
        used = true;
      }else
      if(0 == strcmp(value, "store")){
        this->setPreemptionStrategy(false);
        used = true;
      }else{
        // ignore tag probably for someone else
        //std::cerr << YELLOW("Allocator> Unkown preemption mode!") << std::endl;
      }
    }
    
    return used;
  }
  
  /**
   * \brief Implementation of  Allocator::getController
   */
  AbstractController& Allocator::getController(){
    return *(this->controller);
  }
  
  /**
   * \brief Implementation of Allocator::getWaitInterval
   */
  sc_time* Allocator::getWaitInterval(ReconfigurableComponent* rc){
      return this->waitInterval;
  }
     
  /**
    * \brief Dummy implementation of Allocator::signalDeallocation
    * \sa AbstractAllocator
    */  
  void Allocator::signalDeallocation(bool kill, ReconfigurableComponent* rc){}
  
  /**
    * \brief Dummy implementation of Allocator::signalAllocation
    */
  void Allocator::signalAllocation(ReconfigurableComponent* rc){}
  
  /**
   * \brief Setter to specify if Allocator should use "kill" by preemption
   */
  void Allocator::setPreemptionStrategy(bool kill){
    this->kill = kill;
  }
  
  /**
   * \brief Getter to determine which deallocation mode is used
   */
  bool Allocator::deallocateByKill(){
    return this->kill;
  }
  
  /**
   * \brief Gets the currently controlled reconfigurable Component of instance
   */
  ReconfigurableComponent* Allocator::getManagedComponent(){
    return this->controller->getManagedComponent();
  }

  sc_time Allocator::getSchedulingOverhead(){
    return SC_ZERO_TIME;
  }

}
