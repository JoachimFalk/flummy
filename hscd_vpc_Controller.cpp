#include <hscd_vpc_Controller.h>

namespace SystemC_VPC{
  
  Controller::Controller(const char* name){
    
    strcpy(this->controllerName, name);
    
    char rest[VPC_MAX_STRING_LENGTH];
    int sublength;
    char *secondindex;
    char *firstindex=strchr(name,':');    //':' finden -> ':' trennt key-value Paare 
    while(firstindex!=NULL){
      secondindex=strchr(firstindex+1,':');        //':' überspringen und nächste ':' finden
      if(secondindex!=NULL)
        sublength=secondindex-firstindex;          //Länge bestimmen
      else
        sublength=strlen(firstindex);              
      strncpy(rest,firstindex+1,sublength-1);      //key-value extrahieren
      rest[sublength-1]='\0';
      firstindex=secondindex;                     

      char *key, *value;               // key und value trennen und Property setzen
      value=strstr(rest,"-");
      if(value!=NULL){
        value[0]='\0';
        value++;
        key=rest;
        setProperty(key,value);
      }
    }
    
  }
  
  /**
   * \brief Implementation of Controller::getName()
   */
  char* Controller::getName(){
    
    return this->controllerName;
    
  }
    
  /**
   * \brief Implementation of Controller::setManagedComponent
   */
  void Controller::setManagedComponent(ReconfigurableComponent* managedComponent){
    
    assert(managedComponent != NULL);
    this->managedComponent = managedComponent;
  
  }
  
  /**
   * \brief Implementation of Controller::getManagedComponent
   */
  ReconfigurableComponent* Controller::getManagedComponent(){
    
    return this->managedComponent;
    
  }
  
  /**
   * \brief Implementation of Controller::registerComponent
   */
  void Controller::registerComponent(AbstractComponent* comp){
    // do nothing right now
  }
      
  /**
   * \brief Implementation of Controller::registerMapping
   */
  void Controller::registerMapping(const char* taskName, const char* compName){
    
    // register direct mapping to component
    this->mapping_map_component_ids[taskName] = compName;
    
    // register mapping to configuration
    std::map<std::string, Configuration* > configs;
    configs = this->managedComponent->getConfigurations();
    std::map<std::string, Configuration* >::iterator iter;

    // go through all configurations and find right containing component
    for(iter = configs.begin(); iter != configs.end(); iter++){
      AbstractComponent* comp = (iter->second)->getComponent(compName);
      if(comp != NULL){
        // found component
        this->mapping_map_configs.insert(std::pair<std::string, std::string>(taskName, (iter->second)->getName()));
        // exactly one match possible
        break;
      }  
    }
  }
  
  /**
   * \brief Implementation of  EDFController::setProperty
   */
  void Controller::setProperty(char* key, char* value){

    if(0 == strcmp(key, "mode")){

#ifdef VPC_DEBUG
      std::cerr << VPC_BLUE("Controller> Found input data for preemption mode = ") << value << std::endl;
#endif //VPC_DEBUG
      if(0 == strcmp(value, "kill")){
        this->setPreemptionStrategy(true);
      }else
      if(0 == strcmp(value, "store")){
        this->setPreemptionStrategy(false);
      }else{
        std::cerr << VPC_YELLOW("Controller> Unkown preemption mode!") << std::endl;
      }
    }
    
  }
  
  /**
   * \brief Implementation of Controller::getWaitInterval
   */
  sc_time* Controller::getWaitInterval(){
    
    return this->waitInterval;
    
  }
     
  /**
    * \brief Implementation of Controller::getMappedComponent
    */
  AbstractComponent* Controller::getMappedComponent(ProcessControlBlock* task){
     // determine requied configuration
    std::string configName = this->mapping_map_configs[task->getName()];
    Configuration* conf = this->managedComponent->getConfiguration(configName.c_str());
    // get mapped component from configuration
    AbstractComponent* comp = conf->getComponent((this->mapping_map_component_ids[task->getName()]).c_str());
    
    return comp;
    
   };
  
  /**
    * \brief Dummy implementation of Controller::signalDeallocation
    */  
  void Controller::signalDeallocation(bool kill){}
  
  /**
    * \brief Dummy implementation of Controller::signalAllocation
    */
  void Controller::signalAllocation(){}
  
  /**
    * \brief Implementation of Controller::getMappedConfiguration
    */
  Configuration* Controller::getMappedConfiguration(const char* name){
    
    std::map<std::string, std::string>::iterator iter;
    iter = this->mapping_map_configs.find(name);
    if(iter == this->mapping_map_configs.end()){
      std::cerr << VPC_RED("AbstractController " << this->getName() << "> No mapped configuration found for " << name) << std::endl;
      return NULL;
    }
    
    this->managedComponent->getConfiguration(iter->second.c_str());
    return this->managedComponent->getConfiguration(iter->second.c_str());
  }
  
  /**
   * \brief Setter to specify if controller should use "kill" by preemption
   */
  void Controller::setPreemptionStrategy(bool kill){
    this->kill = kill;
  }
  
  /**
   * \brief Getter to determine which preemption mode is used
   */
  bool Controller::deallocateByKill(){
    return this->kill;
  }
  
  
}
