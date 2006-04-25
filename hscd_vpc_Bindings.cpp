#include "hscd_vpc_Bindings.h"

namespace SystemC_VPC {

  /**
   * SECTION AbstractBinding
   */

  
  AbstractBinding::AbstractBinding(std::string source) : source(source) {}

  AbstractBinding::~AbstractBinding(){}
  
  std::string& AbstractBinding::getSource(){
  
    return this->source;
  
  }


  /**
   * SECTION SimpleBinding
   */

  SimpleBinding::SimpleBinding(std::string source) : AbstractBinding(source), hasTarget(false), target("") {}

  SimpleBinding::~SimpleBinding() {}
  
  void SimpleBinding::reset(){
  
    if(target != ""){
      this->hasTarget = true;
    }
    
  }

  bool SimpleBinding::hasNext(){
  
    return this->hasTarget;
  
  }
  
  std::string SimpleBinding::getNext(){
    
    this->hasTarget = false;
    return this->target;
  
  }

  void SimpleBinding::addBinding(std::string& target){
    
    this->hasTarget = true;
    this->target = target;

  }
  
  /**
   * SECTION Binding
   */

  
  Binding::Binding(std::string source) : AbstractBinding(source) {

    this->iter = this->targets.begin();

  }

  Binding::~Binding() {}
  
  void Binding::reset(){

    this->iter = this->targets.begin();

  }

  bool Binding::hasNext(){
   
    return (this->iter != this->targets.end());
  
  }

  std::string Binding::getNext(){
   
    return *iter++; 
    
  }
  
  void Binding::addBinding(std::string& target){

    this->targets.insert(target);
    
  }
  
}
