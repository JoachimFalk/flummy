#include "hscd_vpc_ConfigurationScheduler.h"

namespace SystemC_VPC{
  
  ConfigurationScheduler::ConfigurationScheduler(AbstractController* controller) 
    : controller(controller), 
      kill(false), 
      waitInterval(NULL) {}
 
  ConfigurationScheduler::~ConfigurationScheduler() {}

  /**
   * \brief Implementation of  ConfigurationScheduler::setProperty
   */
  bool ConfigurationScheduler::setProperty(char* key, char* value){

    bool used = false;

    // check if property relevant for allocator
    if(0 != std::strncmp(key, ALLOCATORPREFIX, strlen(ALLOCATORPREFIX))){
      return used;
    }else{
      key += strlen(ALLOCATORPREFIX);
    }
   
    if(0 == strcmp(key, "mode")){

#ifdef VPC_DEBUG
      std::cerr << VPC_BLUE("ConfigurationScheduler> Found input data for preemption mode = ") << value << std::endl;
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
        //std::cerr << YELLOW("ConfigurationScheduler> Unkown preemption mode!") << std::endl;
      }
    }
    
    return used;
  }
  
  /**
   * \brief Implementation of  ConfigurationScheduler::getController
   */
  AbstractController& ConfigurationScheduler::getController(){
    return *(this->controller);
  }
  
  /**
   * \brief Implementation of ConfigurationScheduler::getWaitInterval
   */
  sc_time* ConfigurationScheduler::getWaitInterval(ReconfigurableComponent* rc){
    
    return this->waitInterval;
    
  }
     
  /**
    * \brief Dummy implementation of ConfigurationScheduler::signalPreemption
    */  
  void ConfigurationScheduler::signalPreemption(bool kill, ReconfigurableComponent* rc){}
  
  /**
    * \brief Dummy implementation of ConfigurationScheduler::signalResume
    */
  void ConfigurationScheduler::signalResume(ReconfigurableComponent* rc){}
  
  /**
   * \brief Setter to specify if ConfigurationScheduler should use "kill" by preemption
   */
  void ConfigurationScheduler::setPreemptionStrategy(bool kill){
    this->kill = kill;
  }
  
  /**
   * \brief Getter to determine which preemption mode is used
   */
  bool ConfigurationScheduler::preemptByKill(){
    return this->kill;
  }
  
  /**
   * \brief Gets the currently controlled reconfigurable Component of instance
   */
  ReconfigurableComponent* ConfigurationScheduler::getManagedComponent(){
    return this->controller->getManagedComponent();
  }
  
}
