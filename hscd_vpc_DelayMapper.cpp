#include "hscd_vpc_DelayMapper.h"

namespace SystemC_VPC {

  DelayMapper::DelayMapper() : base_delay(SC_ZERO_TIME), base_latency(SC_ZERO_TIME) {}

  DelayMapper::~DelayMapper(){}

  void DelayMapper::addDelay(const char* funcname, sc_time delay){

    if(delay < SC_ZERO_TIME){
      delay = SC_ZERO_TIME;
    }

    if(funcname != NULL){
      std::string key(funcname, strlen(funcname));
      this->funcDelays[key] = delay;
    }else{ // interpret call as setting base value
      this->base_delay = delay;
    }

  }

  sc_time DelayMapper::getDelay(const char* funcname) const{
    if(funcname != NULL){
      std::map<std::string, sc_time>::const_iterator iter;
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

  void DelayMapper::addFuncLatency(const char* funcname, sc_time latency){
    if(funcname != NULL){
      std::string key(funcname, strlen(funcname));
      this->funcLatencies[key] = latency;
    }else{
      this->base_latency = latency;
    }
  }

  sc_time DelayMapper::getFuncLatency(const char* funcname) const {
    if(funcname == NULL){
      return this->base_latency;
    }else{
      std::map<std::string, sc_time >::const_iterator iter;
      std::string key(funcname, strlen(funcname));
      iter = this->funcLatencies.find(key);
      if(iter != this->funcLatencies.end()){
        return iter->second;
      }
    }

    return this->base_latency;
  }

  bool DelayMapper::hasLatency(const char* funcname) const {
    if(funcname == NULL){
      return false;
    }else{
      std::string key(funcname, strlen(funcname));
      return (this->funcLatencies.count(key) > 0);
    }
  }

}
