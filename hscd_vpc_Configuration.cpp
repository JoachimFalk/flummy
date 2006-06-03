#include <hscd_vpc_Configuration.h>

#include <sstream>

namespace SystemC_VPC{

  /**
   * IMPLEMENTATION OF Configuration
   */

  // has to start at 1 as 0 is default value for unsigned int!
  unsigned int Configuration::global_id = 1;
  
  /**
   * \brief Creates instance of Configuration
   */
  Configuration::Configuration(const char* name) 
    : activ(false), stored(false){
      
    strcpy(this->configName,name);
    this->id = Configuration::global_id++;
    
    this->loadTime = sc_time(SC_ZERO_TIME);
    this->storeTime = sc_time(SC_ZERO_TIME);
    
  }

  /**
   * \brief Creates instance of Configuration
   */    
  Configuration::Configuration(const char* name, char* loadTime, char* storeTime) 
    : activ(false), stored(false){
      
    strcpy(this->configName,name);
    this->id = Configuration::global_id++;
    
    this->setLoadTime(loadTime);
    this->setStoreTime(storeTime);

#ifdef VPC_DEBUG
    std::cerr << "Configuration " << this->getName() <<"> loadtime set to " << this->getLoadTime() << std::endl;
    std::cerr << "Configuration " << this->getName() <<"> storetime set to " << this->getStoreTime() << std::endl;    
#endif //VPC_DEBUG

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
   * \brief Implementation of Configuration::getID
   */
  unsigned int const& Configuration::getID() const{

    return this->id;

  }
  
  /**
   * \brief Implementation of Configuration::isActiv
   */
  bool Configuration::isActiv() const{

    return this->activ;
    
  }

  /**
   * \brief Getter to check if configuration has been stored
   */
  bool Configuration::isStored() const{
    
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
  AbstractComponent* Configuration::getComponent(std::string name){
  
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
          std::cerr << YELLOW("Configuration " << this->getName() 
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
          std::cerr << YELLOW("Configuration " << this->getName() 
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
          std::cerr << YELLOW("Configuration " << this->getName() 
              << " caculate time to preempt component: " << iter->first) << std::endl;
#endif // VPC_DEBUG
  
          tmp = iter->second->timeToPreempt();
          if(tmp > max){
            max = tmp;
          }
      }
    }

#ifdef VPC_DEBUG
          std::cerr << YELLOW("Configuration " << this->getName() 
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
          std::cerr << YELLOW("Configuration " << this->getName() 
              << " time of resuming: " << max) << std::endl;
#endif // VPC_DEBUG

    return max;
  }

  /**
   * \brief Implementation of Configuration::setStoreTime
   */
  void Configuration::setStoreTime(char* time){
      
#ifdef VPC_DEBUG
    std::cerr << "Configuration> setting store time: " 
        << YELLOW("time = " << time) << std::endl;
#endif //VPC_DEBUG
    
    try{
      this->storeTime = this->createSC_Time(time);
    }catch(InvalidArgumentException& e){
      double timeVal = atof(time);
      this->storeTime = sc_time(timeVal, SC_NS);
    }
#ifdef VPC_DEBUG
    std::cerr << "Configuration> store time set to: " 
        << YELLOW( this->storeTime) << std::endl;
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
  void Configuration::setLoadTime(char* time){
      
#ifdef VPC_DEBUG
    std::cerr << "Configuration> setting load time: " 
        << YELLOW("time = " << time) << std::endl;
#endif //VPC_DEBUG
    
    try{
      this->loadTime = this->createSC_Time(time);
    }catch(InvalidArgumentException& e){
      double timeVal = atof(time);
      this->loadTime = sc_time(timeVal, SC_NS);
    }
    
#ifdef VPC_DEBUG
    std::cerr << "Configuration> load time set to: " 
        << YELLOW(this->loadTime) << std::endl;
#endif //VPC_DEBUG

  }    

  sc_time Configuration::createSC_Time(char* timeString) throw(InvalidArgumentException){
    assert(timeString != NULL);
    double value = -1;
    string unit;

    sc_time_unit scUnit = SC_NS;

    std::stringstream data(timeString);
    int oldPos = data.tellg();
    if(data.good()){
      data >> value;
    }else{
      string msg("Parsing Error: Unknown argument: <");
      msg += timeString;
      msg += "> How to creating a sc_string from?";
      throw InvalidArgumentException(msg);
    }
    if( data.fail() ){
      string msg("Parsing Error: Unknown argument: <");
      msg += timeString;
      msg += "> How to creating a sc_string from?";
      throw InvalidArgumentException(msg);
    }
    if(data.good()){
      data >> unit;
      if(data.fail()){
#ifdef VPC_DEBUG
        std::cerr << "VPCBuilder> No time unit, taking default: SC_NS!" << std::endl;
#endif //VPC_DEBUG
        scUnit = SC_NS;
      }else{
        std::transform (unit.begin(),unit.end(), unit.begin(), (int(*)(int))tolower);
        if(      0==unit.compare(0, 2, "fs") ) scUnit = SC_FS;
        else if( 0==unit.compare(0, 2, "ps") ) scUnit = SC_PS;
        else if( 0==unit.compare(0, 2, "ns") ) scUnit = SC_NS;
        else if( 0==unit.compare(0, 2, "us") ) scUnit = SC_US;
        else if( 0==unit.compare(0, 2, "ms") ) scUnit = SC_MS;
        else if( 0==unit.compare(0, 1, "s" ) ) scUnit = SC_SEC;
      }
    }

    return sc_time(value, scUnit);
  }

  
  /**
   * \brief Implementation of Configuration::getLoadTime
   */
  const sc_time& Configuration::getLoadTime(){
    return this->loadTime;
  }

  /**
   * \brief Implementation of Configuration::getComponentIDIterator
   */
  Configuration::ComponentIDIterator Configuration::getComponentIDIterator(){

    return ComponentIDIterator(this);
  
  }

  /**
   * \brief Implememtation of  Configuration::ComponentIDIterator::ComponentIDIterator
   */
  Configuration::ComponentIDIterator::ComponentIDIterator(Configuration* conf) {

    this->config = conf;
    this->iter = this->config->component_map_by_name.begin();
    
  }

  /**
   * \brief Implementation of Configuration::ComponentIDIterator::hasNext()
   */
  bool Configuration::ComponentIDIterator::hasNext(){
  
    return (this->iter != this->config->component_map_by_name.end());
    
  }

  /**
   * \brief Implementaiton of Configuration::ComponentIDIterator::getNext()
   */
  const std::string Configuration::ComponentIDIterator::getNext() {

    std::string comp = this->iter->first;
    this->iter++;
    return comp;
    
  }
  
}//namespace SystemC_VPC
