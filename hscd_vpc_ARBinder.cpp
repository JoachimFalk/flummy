#include "hscd_vpc_ARBinder.h"

#include "hscd_vpc_ReconfigurableComponent.h"

namespace SystemC_VPC {

  ARBinder::ARBinder() : DynamicBinder() {}

  ARBinder::~ARBinder(){}

  std::pair<std::string, MappingInformation* > ARBinder::performBinding(ProcessControlBlock& task, ReconfigurableComponent* comp)
    throw(UnknownBindingException){

      Binding* b = NULL;
      if(comp == NULL){
        b = task.getBindingGraph().getRoot();
      }else{
        b = task.getBindingGraph().getBinding(comp->basename());
      }
      
      ChildIterator* bIter = b->getChildIterator();
			
			if(!bIter->hasNext()){

				std::string msg = "No binding possibility given for "+ task.getName() +"->?";
				throw UnknownBindingException(msg);

			}
			
			std::string target = "";
			
			std::map<std::string, std::string>::iterator iter;
			iter = this->decisions.find(task.getName());
			// if already decision has been selected take it
			if(iter != this->decisions.end()){
				target = iter->second;
			}else{// else determine fitting target
				
				unsigned int activConfID = 0;
				if(comp->getActivConfiguration() != NULL){
					activConfID = comp->getActivConfiguration()->getID();
				}
				
				// build up helper structs
				std::queue<std::string > noReconf; //<< contains targets with no reconf needed
				std::queue<std::string > reconf; //<< contains targets with reconf required
				
				// check all possibilities
				while(bIter->hasNext()){

					std::string tmp = bIter->getNext()->getID();
					if(this->reconfRequired(task, tmp, comp)){
						noReconf.push(tmp);
					}else{
						reconf.push(tmp);
					}
					
				}
			
				if(noReconf.size() > 0){
					target = noReconf.front();
          noReconf.pop();
					std::pair<std::multimap<std::string, std::pair<std::string, int> >::iterator, 
                    std::multimap<std::string, std::pair<std::string, int> >::iterator> p;
          p = this->boundTo.equal_range(target);
          int currNumTypes = std::distance(p.first, p.second);

					while(noReconf.size() > 0){
						std::string tmp = noReconf.front();
            noReconf.pop();
					 	p = this->boundTo.equal_range(tmp);
						int numTypes = std::distance(p.first, p.second);
						if(numTypes < currNumTypes){
							target = tmp;
							currNumTypes = numTypes;
						}
					}
				}else{
					target = reconf.front();
          reconf.pop();
					std::pair<std::multimap<std::string, std::pair<std::string, int> >::iterator, 
                    std::multimap<std::string, std::pair<std::string, int> >::iterator> p;
          p = this->boundTo.equal_range(target);
          int currNumTypes = std::distance(p.first, p.second);

					while(reconf.size() > 0){
						std::string tmp = reconf.front();
            reconf.pop();
					 	p = this->boundTo.equal_range(tmp);
						int numTypes = std::distance(p.first, p.second);
						if(numTypes < currNumTypes){
							target = tmp;
							currNumTypes = numTypes;
						}
					}
				}
			}
		
			// update internal data
			this->decisions[task.getName()] = target;
			std::pair<std::multimap<std::string, std::pair<std::string, int> >::iterator, 
                std::multimap<std::string, std::pair<std::string, int> >::iterator> p;
      p = this->boundTo.equal_range(target);
     	for(; p.first != p.second; p.first++){
				if(((p.first)->second).first == task.getName()){
					(((p.first)->second).second)++;
					break;
				}
			}
			// need to generate new entry
			if(p.first == p.second){
				this->boundTo.insert(std::pair<std::string, std::pair<std::string, int> >(target, std::pair<std::string, int>(task.getName(), 1)));
			}
			// free allocated binding iterator
			delete bIter;
			
			// no we should have a target ... next just select first mappinginfo for task
			MappingInformationIterator* mIter = b->getMappingInformationIterator(); //this->miMapper->getMappingInformationIterator(task.getName(), target);
			if(mIter->hasNext()){
				MappingInformation* mInfo = mIter->getNext();
				delete mIter;
#ifdef VPC_DEBUG
        std::cerr << "ARBinder> result of binding for " << task.getName() << ":" << task.getPID()is << " is " << target << std::endl;
#endif //VPC_DEBUG
				return std::pair<std::string, MappingInformation* >(target, mInfo);
			}else{
				std::string msg = "No mapping information given for binding possibility "+ target +" for task "+ task.getName();
				throw UnknownBindingException(msg);
			}

    }

	bool ARBinder::reconfRequired(ProcessControlBlock& task, std::string target, ReconfigurableComponent* comp){
		Configuration* c = comp->getActivConfiguration();

#ifdef VPC_DEBUG
    std::cerr << "ARBinder> reconfRequired called for " << task.getName() << " to " << target << std::endl;
#endif //VPC_DEBUG
    
		if(c != NULL 
				&& c->getID() == comp->getController()->getConfigurationMapper()->getConfigForComp(target)){
      //check if sub-comp is reconfigurable component
			ReconfigurableComponent* rc = dynamic_cast<ReconfigurableComponent* >(c->getComponent(target));
			// if != NULL we have a rc
			if(rc != NULL){
				// check for binding fitting activ configuration of rc
				Binding* b = task.getBindingGraph().getBinding(target); //rc->getController()->getBinder()->getBinding(task.getName(), rc);
				ChildIterator* bIter = b->getChildIterator(); //b.reset();
				while(bIter->hasNext()){
          std::string t = bIter->getNext()->getID();
					if(rc->getActivConfiguration() != NULL
							&& rc->getActivConfiguration()->getID() == rc->getController()->getConfigurationMapper()->getConfigForComp(t)){
						return true;
					}
				}
			}else{// else simple component that fits to current configuration
				return true;
			}
		}

		return false;
	}
	
	void ARBinder::signalTaskEvent(ProcessControlBlock* pcb, std::string compID) {

		std::string target = this->decisions[pcb->getName()];
		std::pair<std::multimap<std::string, std::pair<std::string, int> >::iterator, 
		        	std::multimap<std::string, std::pair<std::string, int> >::iterator> p;
		p = this->boundTo.equal_range(target);
		for(; p.first != p.second; p.first++){
			// found entry for task type
			if(((p.first)->second).first == pcb->getName()){
				(((p.first)->second).second)--;
				if(((p.first)->second).second <= 0){
					this->boundTo.erase(p.first);
					this->decisions.erase(pcb->getName());
				}
				break;
			}
		}

	}

}
