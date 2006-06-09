#include "hscd_vpc_MappingInformation.h"

namespace SystemC_VPC {

  /**
   * SECTION MappingInformation
   */

  MappingInformation::MappingInformation()
    : priority(0),
      period(SC_ZERO_TIME),
      deadline(SC_ZERO_TIME),
      dmapper() {}

  MappingInformation::~MappingInformation() {}

  /**
   * \brief Gets period of instance
   */
  void MappingInformation::setPeriod(sc_time period){
    this->period = period;
  }

  sc_time MappingInformation::getPeriod() const{
    return this->period;
  }

  void MappingInformation::setPriority(int priority){
    this->priority = priority;
  }

  int MappingInformation::getPriority() const{
    return this->priority;
  }

  void MappingInformation::setDeadline(sc_time deadline){
    this->deadline = deadline;
  }

  sc_time MappingInformation::getDeadline() const{
    return this->deadline;
  }

  /**
   * \brief Used to register component specific delay to MappingInformation instance
   * \param funcname refers to an function name or may be null for common
   * association only to a component
   * \param delay represents the execution delay needed for execution  on
   * specified component
   */
  void MappingInformation::addDelay(const char* funcname, sc_time delay){

#ifdef VPC_DEBUG
    std::cerr << "MappingInformation> Adding Function Delay ";
    if(funcname != NULL){
      std::cerr << "for function " << funcname;
    }
    std::cerr << " delay " << delay << std::endl;
#endif //VPC_DEBUG

    this->dmapper.addDelay(funcname, delay);
  }

  /**
   * \brief Used to access delay of a PCB instance on a given component
   * \param funcname refers to an optional function name
   */
  sc_time MappingInformation::getDelay(const char* funcname) const{

#ifdef VPC_DEBUG
    std::cerr << "MappingInformation> Delay ";
    if(funcname != NULL){
      std::cerr << "for " << funcname;
    }
    std::cerr << " is " << this->dmapper->getDelay(funcname) << std::endl;
#endif //VPC_DEBUG

    return this->dmapper.getDelay(funcname);
  }
  
  /**
   * \brief Test if an delay is specified for a given component
   * \param funcname refers to an optional function name
   * \return true if a delay is registered else false
   */
  bool MappingInformation::hasDelay(const char* funcname) const {

#ifdef VPC_DEBUG
    std::cerr << "MappingInformation> Request ";
    if(funcname != NULL){
      std::cerr << "for function " << funcname;
    }
    std::cerr << " has special delay ? " << this->dmapper->hasDelay(funcname) << std::endl;
#endif //VPC_DEBUG

    return this->dmapper.hasDelay(funcname);
  }

  void MappingInformation::addFuncLatency(const char* funcname, sc_time delay){

#ifdef VPC_DEBUG
    std::cerr << "MappingInformation> Adding Function Latency";
    if(funcname != NULL){
      std::cerr << " function " << funcname;
    }
    std::cerr << " delay " << delay << std::endl;
#endif //VPC_DEBUG

    this->dmapper.addFuncLatency(funcname, delay);
  }
        
  sc_time MappingInformation::getFuncLatency(const char* funcname) const{

#ifdef VPC_DEBUG
    std::cerr << "MappingInformation> Latency";
    if(funcname != NULL){
      std::cerr << "->" << funcname;
    }
    std::cerr << " is " << this->dmapper->getLatency(comp, funcname) << std::endl;
#endif //VPC_DEBUG

    return this->dmapper.getFuncLatency(funcname);
  }
              
  bool MappingInformation::hasLatency(const char* funcname) const{

#ifdef VPC_DEBUG
    std::cerr << "MappingInformation> Request";
    if(funcname != NULL){
      std::cerr << " with function " << funcname;
    }
    std::cerr << " has special latency ? " << this->dmapper->hasLatency(comp, funcname) << std::endl;
#endif //VPC_DEBUG

    return this->dmapper.hasLatency(funcname);
  }
                    
}

