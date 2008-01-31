#include "hscd_vpc_AbstractBinder.h"

#include "hscd_vpc_AbstractController.h"
#include "hscd_vpc_Binding.h"
#include "hscd_vpc_ProcessControlBlock.h"
#include "hscd_vpc_MappingInformation.h"
#include "hscd_vpc_ProcessEventListener.h"
#include "hscd_vpc_InvalidArgumentException.h"

namespace SystemC_VPC {

   void AbstractBinder::setDirector(AbstractDirector* dir){
      this->director = dir;
   }
   
   AbstractDirector* AbstractBinder::getDirector(){
      return this->director;
   }

  /**
   * \brief Updates data fields of pcb to binding decision
   * This method ensures that at each hierarchy the data fields
   * of a PCB are set to the currently selected binding decision.
   * It has to be called after binding decision has taken place!
   * \param pcb specifies the PCB to update
   * \param mapping refers to the selected MappingInformation to apply
   */
  void LocalBinder::updatePCBData(ProcessControlBlock& pcb, MappingInformation* mapping){

    pcb.setDeadline(mapping->getDeadline());
    pcb.setPeriod(mapping->getPeriod());
    pcb.setPriority(mapping->getPriority());

    pcb.setDelay(mapping->getDelay(pcb.getFuncName()));
    pcb.setLatency(mapping->getFuncLatency(pcb.getFuncName()));
    pcb.setRemainingDelay(pcb.getDelay());

  }

  /**
   * \brief Used to get controlling instance for associated component ID
   * In localized case this method just returns singel aussociated controller
   * for an associated component ID, in global case this method may return different
   * controllers depending on key entry.
   * \throws InvalidArgumentException if specified component ID is unknown.
   */
  AbstractController& LocalBinder::getController(std::string c) throw(InvalidArgumentException){

    std::map<std::string, AbstractController* >::iterator iter;
    iter = ctrls.find(c);
    if(iter != ctrls.end()){
      return *(iter->second);
    }else{
      std::string msg = "No associated controller registered for component id="+ c;
      throw InvalidArgumentException(msg);
    }

  }

  void LocalBinder::registerCompToCtrl(std::string c, AbstractController* ctrl) throw(InvalidArgumentException) {

    std::map<std::string, AbstractController* >::iterator iter;
    iter = this->ctrls.find(c);
    if(iter == this->ctrls.end()){
      this->ctrls[c] = ctrl;
    }else{
      std::string msg = "Duplicated registration for "+ c +" to "+ ctrl->getName() +" ! "+ c +" is already associated to "+ (iter->second)->getName();
      throw InvalidArgumentException(msg);
    }

  }

  /**
   * \brief Implementation of AbstractBinder::resolveBinding
   * Generic implementation of required method:
   *  - perform binding strategy depending on subclasses
   *  - update PCB to made decision
   * \sa AbstractBinder::resolveBinding
   */
  std::string LocalBinder::resolveBinding(ProcessControlBlock& task, ReconfigurableComponent* comp) throw(UnknownBindingException){

    std::pair<std::string, MappingInformation*> decision;
    decision = performBinding(task, comp);
    updatePCBData(task, decision.second);
    return decision.first;

  }


  sc_time LocalBinder::getBindingOverhead(){
    //get Setuptime
//       Decision d;
//       // determine binding of process
//       d.comp = b->getID();
//       AbstractController* myCtrl = comp->getController();
//     if(myCtrl){  
//       d.conf = myCtrl->getConfigurationMapper()->getConfigForComp(d.comp);
//       Configuration* myConf = comp->getConfiguration(d.conf);
//       const sc_time& mytime = myConf->getLoadTime();
//       
//       std::cerr << "setuptime: " << mytime << std::endl;
    
    return SC_ZERO_TIME;
  }

  StaticBinder::StaticBinder() {}

  StaticBinder::~StaticBinder() {}

  /**
   * \brief Used to set Binder specific values
   * Dummy implementation does nothing.
   * \param key specifies the identy of the property
   * \param value specifies the actual value
   * \return true if property value has been used else false
   * \sa AbstractBinder
   */
  bool StaticBinder::setProperty(char* key, char* value){ 
    //check if property relevant for binder
    if(0 != std::strncmp(key, BINDERPREFIX, strlen(BINDERPREFIX))){
      return false;
    }else{
      key += strlen(BINDERPREFIX);
    }
    return false; 
  }


  DynamicBinder::DynamicBinder() {}

  DynamicBinder::~DynamicBinder() {}

  /**
   * \brief Used to set Binder specific values
   * Dummy implementation does nothing.
   * \param key specifies the identy of the property
   * \param value specifies the actual value
   * \return true if property value has been used else false
   * \sa AbstractBinder
   */
  bool DynamicBinder::setProperty(char* key, char* value){
    //check if property relevant for binder
    if(0 != std::strncmp(key, BINDERPREFIX, strlen(BINDERPREFIX))){
      return false;
    }else{
      key += strlen(BINDERPREFIX);
    }
    return false;
  }


}

