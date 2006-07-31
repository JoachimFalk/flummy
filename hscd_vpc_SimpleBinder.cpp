#include "hscd_vpc_SimpleBinder.h"

#include "hscd_vpc_ReconfigurableComponent.h"

namespace SystemC_VPC {

  SimpleBinder::SimpleBinder() : StaticBinder() {}

  SimpleBinder::~SimpleBinder() {}

  std::pair<std::string, MappingInformation* > SimpleBinder::performBinding(ProcessControlBlock& task, ReconfigurableComponent* comp) throw(UnknownBindingException){
    Binding* b = NULL;
    if(comp == NULL){
      b = task.getBindingGraph().getRoot();
    }else{
      b = task.getBindingGraph().getBinding(comp->basename());
    }

    ChildIterator* bIter = b->getChildIterator();
    if(bIter->hasNext()){
			// just take first alternativ
      b = bIter->getNext();
			delete bIter;
      MappingInformationIterator* iter = b->getMappingInformationIterator();
      if(iter->hasNext()){
        MappingInformation* mInfo = iter->getNext();
        delete iter;
        return std::pair<std::string, MappingInformation*>(b->getID(), mInfo);
      }else{
        // also free iterator
        delete iter;
      }
    }

    std::string msg = "No target specified for "+ task.getName() +"->?";
    throw UnknownBindingException(msg);
  }

	void SimpleBinder::signalProcessEvent(ProcessControlBlock* pcb, std::string compID) {}
}
