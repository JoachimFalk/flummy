#include "hscd_vpc_PriorityBinder.h"
#include "hscd_vpc_Controller.h"

namespace SystemC_VPC {

  PriorityBinder::PriorityBinder(Controller* controller, MIMapper* miMapper, PriorityElementFactory* factory) 
    : DynamicBinder(controller, miMapper), factory(factory) {}

  PriorityBinder::~PriorityBinder() {
    
    delete factory;
    
    // clean up management structure
    std::map<std::string, PriorityElement* >::iterator iter;
    for(iter = this->pelems.begin(); iter != this->pelems.end(); iter++){
      delete iter->second;
    }
    
  }

  std::pair<std::string, MappingInformation* > PriorityBinder::performBinding(ProcessControlBlock& task, AbstractComponent* comp) 
    throw(UnknownBindingException){
    
    AbstractBinding& binding = this->getBinding(task.getName());
    MIMapper& mapper = this->getMIMapper();
    
    // reset iterator to the begining
    binding.reset();
    
    //determine best current selectable binding
    PriorityElement* elem=NULL;
    std::string target = "";
    
    // init selection
    if(binding.hasNext()){
      target = binding.getNext();
      elem = this->pelems[target];
    }else{
      std::string msg = task.getName() +"->?";
      throw UnknownBindingException(msg);
    }
    
    PriorityElement* tmpElem=NULL;
    std::string tmpTarget;

    while(binding.hasNext()){
       tmpTarget = binding.getNext();
       tmpElem = this->pelems[tmpTarget];
       // if priority of tmpElem is higher then current select switch
       if(*tmpElem > *elem){
         target = tmpTarget;
         elem = tmpElem;
       }
    }
    
    // update priority element by offering selectable mapping infos of binding
    MappingInformationIterator* iter = mapper.getMappingInformationIterator(target);
    MappingInformation* mInfo = elem->addMappingData(*iter);
    delete iter;
    
    return std::pair<std::string, MappingInformation*>(target, mInfo);
  }

  void PriorityBinder::registerBinding(std::string src, std::string target){

    DynamicBinder::registerBinding(src, target);
    
    // initialize additional management structures
    std::map<std::string, PriorityElement* >::iterator iter;
    iter = this->pelems.find(target);
    // check if no a PriorityElement already exists
    if(iter == this->pelems.end()){
      PriorityElement* elem = this->factory->createInstance();
      this->pelems[target] = elem;
    }

  }

  void PriorityBinder::signalTaskEvent(ProcessControlBlock* pcb){

    Decision d = this->getController().getDecision(pcb->getPID());
    MIMapper& mapper = this->getMIMapper();
    MappingInformationIterator* mIter = mapper.getMappingInformationIterator(d.comp);
    PriorityElement* elem = this->pelems[d.comp];
    
    elem->removeMappingData(*mIter);

    delete mIter;
  }
  
}
