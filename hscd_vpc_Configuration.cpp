#include <hscd_vpc_Configuration.h>

namespace SystemC_VPC{

  /**
   * IMPLEMENTATION OF Configuration
   */

  /**
   * \brief Creates instance of Configuration
   */
  Configuration::Configuration(const char* name) 
    : activ(false), stored(false){
      
    strcpy(this->configName,name);
    this->loadTime = sc_time(SC_ZERO_TIME);
    this->storeTime = sc_time(SC_ZERO_TIME);
    
  }

  /**
   * \brief Creates instance of Configuration
   */    
  Configuration::Configuration(const char* name, const char* loadTime, const char* storeTime) 
    : activ(false), stored(false){
      
    strcpy(this->configName,name);
    this->setLoadTime(loadTime);
    this->setStoreTime(storeTime);
      
  }
  
  /**
   * \brief Deletes instance of Configuration
   */
  Configuration::~Configuration(){
  
    std::map<std::string, AbstractComponent* >::iterator iter;
    
    for(iter = this->component_map_by_name.begin();
      iter != this->component_map_by_name.end(); iter ++){
      
      delete iter->second;
      
    }
    
    this->component_map_by_name.clear();
  }
  
  /**
   * \brief Implementation of Configuration::getName
   */
  char* Configuration::getName(){
    
    return this->configName;
    
  }

  /**
   * \brief Implementation of Configuration::isActiv
   */
  bool Configuration::isActiv(){

    return this->activ;
    
  }

  /**
   * \brief Getter to check if configuration has been stored
   */
  bool Configuration::isStored(){
    
    return this->stored;

  }
  
  /**
   * \brief Setter to set if configuration has been stored
   */
  void Configuration::setStored(bool stored){
  
    this->stored = stored;
    
  }
    
  /**
   * \brief Implementation of Configuration::addComponent
   */
  bool Configuration::addComponent(const char* name, AbstractComponent* comp){
       
    assert(name != NULL);
    assert(comp != NULL);
       
    // check if already a component is known under given name
    if(this->component_map_by_name.find(name) 
      != this->component_map_by_name.end()){
      return false;
    }
      
    this->component_map_by_name[name] = comp;
    // deactivate component by standard
    comp->preempt(true); //setActiv(false);
    return true;
    
  }
   
  /**
   * \brief Implementation of Configuration::getComponent
   */
  AbstractComponent* Configuration::getComponent(const char* name){
  
    assert(name != NULL);
    
    std::map<std::string, AbstractComponent* >::iterator iter;
    iter = this->component_map_by_name.find(name);
    if(iter != this->component_map_by_name.end()){
      return iter->second;
    }
    
    return NULL;
    
  }
    
  /**
   * \brief Implementation of Configuration::preempt
   */
  void Configuration::preempt(bool kill){
    
    if(this->isActiv()){
      
      std::map<std::string, AbstractComponent* >::iterator iter;
      
      for(iter = this->component_map_by_name.begin(); 
        iter != this->component_map_by_name.end(); iter++){
  
#ifdef VPC_DEBUG
          std::cerr << VPC_YELLOW("Configuration " << this->getName() 
              << " trying to preempt component: " << iter->first << " with kill=" << kill) << std::endl;
#endif // VPC_DEBUG
  
          iter->second->preempt(kill);

      }
      
      this->activ = false;
    }
    
  }
  
  /**
   * \brief Implementation of Configuration::resume
   */
  void Configuration::resume(){
    
    if(!this->isActiv()){  
      
      std::map<std::string, AbstractComponent*>::iterator iter;
      for(iter = this->component_map_by_name.begin(); 
        iter != this->component_map_by_name.end(); iter++){

#ifdef VPC_DEBUG
          std::cerr << VPC_YELLOW("Configuration " << this->getName() 
              << " trying to resume component: " << iter->first) << std::endl;
#endif // VPC_DEBUG
  
        iter->second->resume(); 
      }
      
      this->activ = true;
    }
    
  }

  /**
   * \brief Implementation of Configuration::timeToPreempt
   */    
  sc_time Configuration::timeToPreempt(){
    
    sc_time max(SC_ZERO_TIME);
    sc_time tmp;
    
    if(this->isActiv()){
      std::map<std::string, AbstractComponent* >::iterator iter;
      
      for(iter = this->component_map_by_name.begin(); 
        iter != this->component_map_by_name.end(); iter++){
  
#ifdef VPC_DEBUG
          std::cerr << VPC_YELLOW("Configuration " << this->getName() 
              << " caculate time to preempt component: " << iter->first) << std::endl;
#endif // VPC_DEBUG
  
          tmp = iter->second->timeToPreempt();
          if(tmp > max){
            max = tmp;
          }
      }
    }

#ifdef VPC_DEBUG
          std::cerr << VPC_YELLOW("Configuration " << this->getName() 
              << " time of preemption: " << max) << std::endl;
#endif // VPC_DEBUG
    
    return max;
  }

  /**
   * \brief Implementation of Configuration::timeToResume
   */
  sc_time Configuration::timeToResume(){
    
    sc_time max(SC_ZERO_TIME);
    sc_time tmp;
    
    if(!this->isActiv()){
        
      std::map<std::string, AbstractComponent*>::iterator iter;
      for(iter = this->component_map_by_name.begin(); iter != this->component_map_by_name.end(); iter++){
        tmp = iter->second->timeToResume();
        if(tmp > max){
          max = tmp;
        } 
      }
    }

#ifdef VPC_DEBUG
          std::cerr << VPC_YELLOW("Configuration " << this->getName() 
              << " time of resuming: " << max) << std::endl;
#endif // VPC_DEBUG

    return max;
  }

  /**
   * \brief Implementation of Configuration::setStoreTime
   */
  void Configuration::setStoreTime(const char* time){
      
    double timeVal = atof(time);

#ifdef VPC_DEBUG
    std::cerr << "Configuration> setting store time: " 
        << VPC_YELLOW("time = " << time) << std::endl;
#endif //VPC_DEBUG
    
    this->storeTime = sc_time(timeVal, SC_NS);
    
#ifdef VPC_DEBUG
    std::cerr << "Configuration> store time set to: " 
        << VPC_YELLOW( this->storeTime) << std::endl;
#endif //VPC_DEBUG
  }    

  /**
   * \brief Implementation of Configuration::getStoreTime
   */
  const sc_time& Configuration::getStoreTime(){
    return this->storeTime;
  }

  /**
   * \brief Implementation of Configuration::setLoadTime
   */
  void Configuration::setLoadTime(const char* time){
      
    double timeVal = atof(time);

#ifdef VPC_DEBUG
    std::cerr << "Configuration> setting load time: " 
        << VPC_YELLOW("time = " << time) << std::endl;
#endif //VPC_DEBUG
    
    this->loadTime = sc_time(timeVal, SC_NS);

#ifdef VPC_DEBUG
    std::cerr << "Configuration> load time set to: " 
        << VPC_YELLOW(this->loadTime) << std::endl;
#endif //VPC_DEBUG

  }    

  /**
   * \brief Implementation of Configuration::getLoadTime
   */
  const sc_time& Configuration::getLoadTime(){
    return this->loadTime;
  }

}//namespace SystemC_VPC
