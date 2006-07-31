#include "hscd_vpc_RRBinder.h"

#include "hscd_vpc_ReconfigurableComponent.h"

namespace SystemC_VPC {
  
  RRBinder::RRBinder() : DynamicBinder() {}

  RRBinder::~RRBinder() {}

  std::pair<std::string, MappingInformation* > RRBinder::performBinding(ProcessControlBlock& task, ReconfigurableComponent* comp)
		throw(UnknownBindingException) {

			ChildIterator* bIter = NULL;
			//check if binding iter already exists
			std::map<std::string, ChildIterator* >::iterator iter;
			iter = this->possibilities.find(task.getName());
			if(iter == this->possibilities.end()){
				Binding* binding = NULL;
        if(comp == NULL){
          binding = task.getBindingGraph().getRoot();
        }else{
          binding = task.getBindingGraph().getBinding(comp->basename());
        }

				bIter = binding->getChildIterator();
				this->possibilities[task.getName()] = bIter;
			}else{
				bIter = (iter->second);
			}
			
			// reset binding iterator if end of possibilites reached
			if(!bIter->hasNext()){
				bIter->reset();
			}

			// check if binding possibility exists
			if(bIter->hasNext()){
				Binding* b = bIter->getNext();
				MappingInformationIterator* iter = b->getMappingInformationIterator();
				if(iter->hasNext()){
					MappingInformation* mInfo = iter->getNext();
					delete iter;
					return std::pair<std::string, MappingInformation* >(b->getID(), mInfo);
				}else{
					// also free iter
					delete iter;
				}
			}

			std::string msg = "No binding possibility given for "+ task.getName() +"->?";
			throw UnknownBindingException(msg);
		}

  void RRBinder::signalProcessEvent(ProcessControlBlock* pcb, std::string CompID){
    // do nothing
  }

}
