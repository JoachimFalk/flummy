#include "hscd_vpc_ConfigurationPool.h"

namespace SystemC_VPC {

  ConfigurationPool::ConfigurationPool() {}

  ConfigurationPool::~ConfigurationPool() {
  
    std::map<unsigned int, Configuration* >::iterator iter;
    for(iter = this->configs.begin(); iter != this->configs.end(); iter++){
      delete iter->second;
    }
    
  }

  void ConfigurationPool::addConfiguration(Configuration* config){

#ifdef VPC_DEBUG
    std::cerr << "ConfigurationPool> adding Configuration "<< config->getName() << " with ID " << config->getID(); 
#endif //VPC_DEBUG

    this->configs[config->getID()] = config;
    
  }

  Configuration* ConfigurationPool::getConfigByID(unsigned int confID) throw(InvalidArgumentException) {

#ifdef VPC_DEBUG    
    std::cerr << "ConfigurationPool> getting Configuration with ID " << confID << std::endl;
#endif //VPC_DEBUG
    
    std::map<unsigned int, Configuration* >::iterator iter;
    iter = this->configs.find(confID);
    if(iter != this->configs.end()){
      return iter->second;
    }

    std::string msg = "Unkown configuration id " + confID;
    throw InvalidArgumentException(msg);
    
  }

  unsigned int ConfigurationPool::getConfigForComp(std::string compName) throw(InvalidArgumentException) {

#ifdef VPC_DEBUG
    std::cerr << "ConfigurationPool> getConfigForComp(" << compName << ")" << std::endl;  
    std::cerr << "ConfigurationPool> mapping map size " << this->comp_to_config.size() << std::endl;
    std::map<std::string, unsigned int>::iterator debug;
    for(debug = this->comp_to_config.begin();
        debug != this->comp_to_config.end();
        debug++){
      std::cerr << debug->first << "->" << debug->second << std::endl;
    }
#endif //VPC_DEBUG
    
    unsigned int mapping = this->comp_to_config[compName];

    // if mapping is 0 as default value there exists no configuration for
    if(mapping == 0){
      std::string msg = "No configuration mapping known for " + compName;
      throw InvalidArgumentException(msg);
    }

    return mapping;
  }
 
  void ConfigurationPool::registerConfiguration(Configuration* conf){
#ifdef VPC_DEBUG   
    std::cerr << "ConfigurationPool> registerConfiguration for " << conf->getName() << std::endl;
#endif //VPC_DEBUG
    
    Configuration::ComponentIDIterator iter = conf->getComponentIDIterator();
    while(iter.hasNext()){
      std::string comp = iter.getNext();

#ifdef VPC_DEBUG
      std::cerr << "ConfigurationPool> adding mapping " << comp << "->" << conf->getID() << std::endl;
#endif //VPC_DEBUG

      this->comp_to_config[comp] = conf->getID();
    }  
    
  }

  bool ConfigurationPool::setProperty(char* key, char* value){
    return false;
  }

}
