#include "hscd_vpc_PriorityBinder.h"

#include "hscd_vpc_ReconfigurableComponent.h"

namespace SystemC_VPC {

  PriorityBinder::PriorityBinder(PriorityElementFactory* factory) 
    : DynamicBinder(), factory(factory) {}

  PriorityBinder::~PriorityBinder() {
    
    delete factory;
    
    // clean up management structure
    std::map<std::string, PriorityElement* >::iterator iter;
    for(iter = this->pelems.begin(); iter != this->pelems.end(); iter++){
      delete iter->second;
    }
    
  }

  std::pair<std::string, MappingInformation* > PriorityBinder::performBinding(ProcessControlBlock& task, ReconfigurableComponent* comp) 
    throw(UnknownBindingException){
     
    Binding* binding = NULL;
    if(comp == NULL){
      binding = task.getBindingGraph().getRoot();
    }else{
      binding = task.getBindingGraph().getBinding(comp->basename());
    }

   	ChildIterator* bIter = binding->getChildIterator(); 
    
    //determine best current selectable binding
    PriorityElement* elem=NULL;
    std::string target = "";
    
    // init selection
    if(bIter->hasNext()){
      target = bIter->getNext()->getID();
#ifdef VPC_DEBUG
      std::cerr << "PriorityElement> intial target is " << target << std::endl;
#endif //VPC_DEBUG
      elem = this->getPElem(target);
    }else{
      std::string msg = task.getName() +"->?";
      throw UnknownBindingException(msg);
    }
    
    PriorityElement* tmpElem=NULL;
    std::string tmpTarget;

    // based on the made decision select highest prior element
    while(bIter->hasNext()){
       tmpTarget = bIter->getNext()->getID();
       tmpElem = this->getPElem(tmpTarget);
       // if priority of tmpElem is higher then current select switch
       if(*tmpElem > *elem){
#ifdef VPC_DEBUG
         std::cerr << "PriorityBinder> higher prior element found! New target is " << tmpTarget << std::endl;
#endif //VPC_DEBUG
         target = tmpTarget;
         elem = tmpElem;
       }
    }
    
    // update priority element by offering selectable mapping infos of binding
    MappingInformationIterator* iter = task.getBindingGraph().getBinding(target)->getMappingInformationIterator(); //mapper.getMappingInformationIterator(task.getName(), target);
    MappingInformation* mInfo = elem->addMappingData(*iter);
    delete iter;
    
    return std::pair<std::string, MappingInformation*>(target, mInfo);
  }

  PriorityElement* PriorityBinder::getPElem(std::string target){
    
    // initialize additional management structures
    std::map<std::string, PriorityElement* >::iterator iter;
    iter = this->pelems.find(target);
    // check if no a PriorityElement already exists
    if(iter == this->pelems.end()){
      PriorityElement* tmp = this->factory->createInstance();
      this->pelems[target] = tmp;
      return tmp;
    }else{
			return (iter->second);
		}
    
  }

  void PriorityBinder::signalTaskEvent(ProcessControlBlock* pcb, std::string compID){

    MappingInformationIterator* mIter = pcb->getBindingGraph().getBinding(compID)->getMappingInformationIterator();
    PriorityElement* elem = this->getPElem(compID);
    
    elem->removeMappingData(*mIter);

    delete mIter;
  }
  
}
