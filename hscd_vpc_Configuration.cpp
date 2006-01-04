#include <hscd_vpc_Configuration.h>

namespace SystemC_VPC{

	/**
	 * IMPLEMENTATION OF Configuration
	 */

	/**
	 * \brief Creates instance of Configuration
	 */
	Configuration::Configuration(const char* name){
			
		strcpy(this->configName,name);
		
	}
		
	Configuration::~Configuration(){
	
		std::map<std::string, AbstractComponent* >::iterator iter;
		
		for(iter = this->component_map_by_name.begin();
			iter != this->component_map_by_name.end(); iter ++){
			
			delete iter->second;
			
		}
		
		this->component_map_by_name.clear();
	}
	
	/**
	 * \brief Implementation of Configuration::getName
	 */
	char* Configuration::getName(){
		
		return this->configName;
		
	}

	/**
	 * \brief Implementation of Configuration::addComponent
	 */
	bool Configuration::addComponent(const char* name, AbstractComponent* comp){
		   
		assert(name != NULL);
		assert(comp != NULL);
		   
		// check if already a component is known under given name
		if(this->component_map_by_name.find(name) 
			!= this->component_map_by_name.end()){
			return false;
		}
		  
		this->component_map_by_name[name] = comp;
		comp->preempt();
		return true;
		
	}
	 
	/**
	 * \brief Implementation of Configuration::getComponent
	 */
	AbstractComponent* Configuration::getComponent(const char* name){
	
		assert(name != NULL);
		
		std::map<std::string, AbstractComponent* >::iterator iter;
		iter = this->component_map_by_name.find(name);
		if(iter != this->component_map_by_name.end()){
			return iter->second;
		}
		
		return NULL;
		
	}
	  
	/**
	 * \brief Implementation of Configuration::minTimeToIdle
	 */
	sc_time* Configuration::minTimeToIdle(){
		
		// maximum of all minimum times of contained components
		sc_time* maxOfmins = new sc_time(SC_ZERO_TIME);
		
		std::map<std::string, AbstractComponent*>::iterator iter;
		for(iter = this->component_map_by_name.begin();
			iter != this->component_map_by_name.end(); iter++){
			
			sc_time* currMin = iter->second->minTimeToIdle();
			if(*currMin > *maxOfmins){
				delete maxOfmins;
				maxOfmins = currMin;
			}
		
		}
		
		return maxOfmins;
	}
	
	/**
	 * \brief Implementation of Configuration::preemptComponents
	 */
	void Configuration::preemptComponents(){
		
		std::map<std::string, AbstractComponent* >::iterator iter;
		
		for(iter = this->component_map_by_name.begin(); 
			iter != this->component_map_by_name.end(); iter++){

#ifdef VPC_DEBUG
				std::cerr << GREEN("Configuration " << this->getName() 
						<< " trying to preempt component: " << iter->first) << std::endl;
#endif // VPC_DEBUG

				iter->second->preempt();
		}
	
	}
	
	/**
	 * \brief Implementation of Configuration::resumeComponents
	 */
	void Configuration::resumeComponents(){
	
		std::map<std::string, AbstractComponent*>::iterator iter;
		for(iter = this->component_map_by_name.begin(); iter != this->component_map_by_name.end(); iter++){
			iter->second->resume();
		}
	
	}

}//namespace SystemC_VPC
