#include "hscd_vpc_ConfigurationScheduler.h"

namespace SystemC_VPC{
  
  ConfigurationScheduler::ConfigurationScheduler(AbstractController* controller, MIMapper* miMapper) 
    : controller(controller), 
      miMapper(miMapper),
      kill(false), 
      waitInterval(NULL) {}
 
  ConfigurationScheduler::~ConfigurationScheduler() {}

  /**
   * \brief Implementation of  ConfigurationScheduler::setProperty
   */
  bool ConfigurationScheduler::setProperty(char* key, char* value){

    bool used = false;
    
    if(0 == strcmp(key, "mode")){

#ifdef VPC_DEBUG
      std::cerr << BLUE("ConfigurationScheduler> Found input data for preemption mode = ") << value << std::endl;
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
   * \brief Impelementation of ConfigurationScheduler::getMIMapper
   */
  MIMapper& ConfigurationScheduler::getMIMapper(){
    return *(this->miMapper);
  }

  /**
   * \brief Implementation of ConfigurationScheduler::getWaitInterval
   */
  sc_time* ConfigurationScheduler::getWaitInterval(){
    
    return this->waitInterval;
    
  }
     
  /**
    * \brief Dummy implementation of ConfigurationScheduler::signalPreemption
    */  
  void ConfigurationScheduler::signalPreemption(bool kill){}
  
  /**
    * \brief Dummy implementation of ConfigurationScheduler::signalResume
    */
  void ConfigurationScheduler::signalResume(){}
  
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
