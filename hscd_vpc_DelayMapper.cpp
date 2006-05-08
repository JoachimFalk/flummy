#include "hscd_vpc_DelayMapper.h"

namespace SystemC_VPC {

  DelayMapper::DelayMapper(double base_delay) : base_delay(base_delay) {}

  DelayMapper::~DelayMapper(){}
  
  void DelayMapper::addDelay(const char* funcname, double delay){
    
    if(delay < 0){
      delay = 0;
    }
    
    if(funcname != NULL){
      std::string key(funcname, strlen(funcname));
      this->funcDelays[key] = delay;
    }else{ // interpret call as setting base value
      this->base_delay = delay;
    }
    
  }

  double DelayMapper::getDelay(const char* funcname) const{
    if(funcname != NULL){
      std::map<std::string, double>::const_iterator iter;
      //std::string key(funcname, strlen(funcname));
      iter = this->funcDelays.find(funcname);
      if(iter != this->funcDelays.end()){
        return iter->second;
      }
    }

    return this->base_delay;
  }

  bool DelayMapper::hasDelay(const char* funcname) const{
    if(funcname == NULL){
      return false;
    }else{
      std::string key(funcname, strlen(funcname));
      return (this->funcDelays.count(key) > 0);
    }
  }

}
