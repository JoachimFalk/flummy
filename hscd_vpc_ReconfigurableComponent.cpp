#include <hscd_vpc_ReconfigurableComponent.h>
#include <hscd_vpc_datatypes.h>
#include <hscd_vpc_Director.h>

#include <values.h>

namespace SystemC_VPC{
	/**
	 * IMPLEMENTATION OF ReconfigurableComponent
	 */
	
	/**
	 * \brief An implementation of AbstractComponent used together with passive actors and global SMoC v2 Schedulers.
	 */
	ReconfigurableComponent::ReconfigurableComponent(sc_module_name name, const char* type): sc_module(name) {
		
		SC_THREAD(schedule_thread);
		strcpy(this->componentName, name);
		this->setController(type);
		
		if(!this->isActiv()){
#ifdef VPC_DEBUG
			std::cerr << GREEN(this->getName() << "> Activating") << std::endl;
#endif //VPC_DEBUG
			this->setActiv(true);
		}
	}
	
	/**
	 * \brief Implementation of destructor of ReconfigurableComponent
	 */
	ReconfigurableComponent::~ReconfigurableComponent(){}
	
	/**
	 * \brief Implementation of ReconfigurableComponent::schedule_thread()
	 */
	void ReconfigurableComponent::schedule_thread(){
		/*******************************************
		 ** 
		 ** while(true){
		 ** 			wait for new tasks;
		 ** 			pass Tasks to Controller;
		 ** 			request delegatable tasks from Controller;
		 ** 			delegate them by using informations from Controller;
		 **	}
		 **
		 *******************************************/
		 
		// time which NonPreemptivController has to wait at least to perform reconfiguration
		sc_time* minTimeToWait = NULL;
		bool newTasksDuringLoad = false;
		
		while(true){
			// if there are still tasks remaining to pass to controller dont wait for notification
			if( !newTasksDuringLoad && this->isActiv() ){	
#ifdef VPC_DEBUG
				std::cerr << RED("ReconfigurableComponent "<< this->getName() <<"> going to wait at time: ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG
									
				// check if ReconfigurableComponent has to wait for configuration to "run to completion"
				if(minTimeToWait == NULL){
					
#ifdef VPC_DEBUG
					std::cerr << RED("ReconfigurableComponent "<< this->getName() <<"> waiting for new tasks.") << endl;
#endif //VPC_DEBUG

					wait(notify_schedule_thread | notify_preempt);
				
				}else{
				
#ifdef VPC_DEBUG
					std::cerr << RED("ReconfigurableComponent "<< this->getName() <<"> waiting for redelegation in: ") << minTimeToWait->to_default_time_units() << endl;
#endif //VPC_DEBUG
	
					this->wait(*minTimeToWait, notify_schedule_thread | notify_preempt);
					delete minTimeToWait;
					minTimeToWait = NULL;
					
				}
			}	
			
#ifdef VPC_DEBUG
			std::cerr << RED("ReconfigurableComponent "<< this->getName() <<"> got notified at time: ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

			// if component is preempted wait till resume
			if( !this->isActiv() ){

#ifdef VPC_DEBUG
				std::cerr << RED("ReconfigurableComponent "<< this->getName() <<"> not activ going to sleep at time: ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

				this->wait(notify_resume);
				
#ifdef VPC_DEBUG
				std::cerr << RED("ReconfigurableComponent "<< this->getName() <<"> awoke at time: ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

			}

			// inform controller about new tasks
			this->controller->addTasksToSchedule(this->newTasks);

#ifdef VPC_DEBUG
				std::cerr << RED("ReconfigurableComponent "<< this->getName() <<"> came here !") << sc_simulation_time() << endl;
#endif //VPC_DEBUG
			
			// delegate all processable tasks
			// points to currently viewed task to delegate
			std::pair<p_struct*, const char* > currTask;
			// points to component to delegate task to
			AbstractComponent* currComp;
			
			while(this->controller->hasTaskToProcess()){

#ifdef VPC_DEBUG
				std::cerr << RED("ReconfigurableComponent "<< this->getName() <<"> came here !") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

				currTask = this->controller->getNextTask();
				currComp = this->controller->getMappedComponent(currTask.first);
				currComp->compute(currTask.first, currTask.second);
			
			}


#ifdef VPC_DEBUG
				std::cerr << RED("ReconfigurableComponent "<< this->getName() <<"> came here !") << sc_simulation_time() << endl;
#endif //VPC_DEBUG
			
			// check if new configuration has to be loaded
			// points to required configuration for a given task
			Configuration* nextConfig;
			nextConfig = this->controller->getNextConfiguration();
			// there is new configuration load it
			if(nextConfig != NULL){
				
				// perform loading
				this->loadNewConfiguration(nextConfig, this->controller->getReconfigurationTime());
				
				//check if new task register during loadphase which may have caused wait
				if(this->newTasks.size() > 0){
					newTasksDuringLoad = true;
				}
			}
			
    		// check if controller request special interval to be called next time
    		minTimeToWait = this->controller->getNotifyInterval();
		}
	}
	
    /**
	 * \brief Implementation of ReconfigurableComponent::addConfiguration
	 */
	void ReconfigurableComponent::addConfiguration(const char* name, Configuration* config){
		
		assert(name != NULL);
		assert(config != NULL);
		/*
		if(this->config_map_by_name.size() == 0){
			this->activConfiguration = config;
		}*/
		this->config_map_by_name[name] = config;

	}
	
	/**
	 * \brief Implementation of ReconfigurableComponent::getConfiguration
	 */
	Configuration* ReconfigurableComponent::getConfiguration(const char* name){
		
		assert(name != NULL);
		
		map<std::string, Configuration* >::iterator iter;
		
		iter = this->config_map_by_name.find(name);
		if(iter != this->config_map_by_name.end()){
			return iter->second;
		}

		return NULL;

	}
	
	/**
	 * \brief Implementation of ReconfigurableComponent::getConfigurations
	 */
	std::map<std::string, Configuration*>& ReconfigurableComponent::getConfigurations(){
		    
		return this->config_map_by_name;

	}

	/**
	 * \brief Implementation of ReconfigurableComponent::getActivConfiguraton
	 */
	Configuration* ReconfigurableComponent::getActivConfiguration(){
	    
		return this->activConfiguration;

	}
	
	/**
	 * \brief Implementation of ReconfigurableComponent::setActivConfiguration
	 */
	void ReconfigurableComponent::setActivConfiguration(const char* name){

		assert(name != NULL);
	
		std::map<std::string, Configuration* >::iterator newConfig;
		newConfig = this->config_map_by_name.find(name);

		//check if request configuration exists
		if(newConfig != this->config_map_by_name.end() 
			&& this->activConfiguration != newConfig->second){
			
			// stop currently running components
			if(this->activConfiguration != NULL){
				this->activConfiguration->preemptComponents();
			}
			this->activConfiguration = newConfig->second;
			
			// activate new configration
			this->activConfiguration->resumeComponents();
		}

	}
	
	/**
	 * \brief Implementation of ReconfigurableComponent::getController
	 */
	AbstractController* ReconfigurableComponent::getController(){
		
		return this->controller;
		
	}
	
	/**
	 * \brief Implementation of ReconfigurableComponent::setController
	 */
	void ReconfigurableComponent::setController(const char* controllertype){
		// TODO IMPLEMENT WHEN CONTROLLER IS IMPLEMENTED
		
		if(0==strncmp(controllertype,"NonPreemptivController",strlen("NonPreemptivController"))){
			this->controller = new NonPreemptivController(this->componentName);			
		}else 
		if(0==strncmp(controllertype,"PreemptivController",strlen("PreemptivController"))){
			this->controller = new PreemptivController(this->componentName);
		}else{
			this->controller = new NonPreemptivController(this->componentName);
		}
		
		// register backward to controller
		this->controller->setManagedComponent(this);
	}
 
	/**
	 * \brief Implementation of ReconfigurableComponent::minTimeToIdle
	 */
	sc_time* ReconfigurableComponent::minTimeToIdle(){
     
		if(this->activConfiguration == NULL){
#ifdef VPC_DEBUG
			std::cerr << BLUE("ReconfigurableComponent> minTimeToIdle: No Configuration set!!!?") << std::endl;
#endif //VPC_DEBUG
			return new sc_time(SC_ZERO_TIME);
		}else{
#ifdef VPC_DEBUG
			std::cerr << BLUE("ReconfigurableComponent> minTimeToIdle: Configuration " 
						<< this->activConfiguration->getName()) << std::endl;
#endif //VPC_DEBUG
			return this->activConfiguration->minTimeToIdle();
		}

	}

	/**
	 * \brief Implementation of ReconfigurableComponent::preempt
	 */
	void ReconfigurableComponent::preempt(){
	    
		// only preempt activ component
		if(this->isActiv()){
			if(this->activConfiguration != NULL){
				this->activConfiguration->preemptComponents();
			}
			this->setActiv(false);   
			this->notify_preempt.notify();   
		}

	}
	
	/**
	 * \brief Implementation of ReconfigurableComponent::resume
	 */
	void ReconfigurableComponent::resume(){
		    
		// only resume preempted component
		if(!this->isActiv()){
			if(this->activConfiguration != NULL){
				this->activConfiguration->resumeComponents();
			}
			this->setActiv(true);
			this->notify_resume.notify();
		}
   
	}
	
	/**
	 * \brief An implementation of AbstractComponent::compute(const char *, const char *, CoSupport::SystemC::Event).
	 */
	void ReconfigurableComponent::compute( const char* name, const char* funcname, CoSupport::SystemC::Event* end){
		
		// send compute request to controller
		p_struct* pcb = Director::getInstance().getProcessControlBlock(name);
		pcb->blockEvent = end;
		this->compute(pcb, funcname);
		
	}
	    
	/**
	 * \brief An implementation of AbstractComponent::compute(const char *, CoSupport::SystemC::Event).
	 */
	void ReconfigurableComponent::compute( const char *name, CoSupport::SystemC::Event *end){
		
		this->compute(name, "", end);
		
	}
	
	/**
	 * \brief An implementation of AbstractComponent::compute(p_struct*, const char *).
	 */
	void ReconfigurableComponent::compute(p_struct* pcb, const char *funcname){
		
		this->newTasks.push_back(std::pair<p_struct*, const char*>(pcb, funcname));
		this->notify_schedule_thread.notify();
		
		if(pcb->blockEvent == NULL){

#ifdef VPC_DEBUG
		std::cerr << GREEN("ReconfigurableComponent " << this->getName() << "> No event to notify register -> init activ waiting for ") 
				  << pcb->name << std::endl;		
#endif //VPC_DEBUG

			pcb->blockEvent = new CoSupport::SystemC::Event();
			CoSupport::SystemC::wait(*(pcb->blockEvent));

#ifdef VPC_DEBUG
		std::cerr << GREEN("ReconfigurableComponent " << this->getName() << "> returned after activ waiting for ")
				  << pcb->name << std::endl;		
#endif //VPC_DEBUG

			delete pcb->blockEvent;
			pcb->blockEvent = NULL;
			// return
		}
	}
		
	/**
	 * \brief Used to create the Tracefiles.
	 *
	 * To create a vcd-trace-file in SystemC all the signals to 
	 * trace have to be in a "global" scope. The signals have to 
	 * be created in elaboration phase (before first sc_start).
	 */
	void ReconfigurableComponent::informAboutMapping(std::string module){
		// TODO IMPLEMENT
	}

	/**
	 * \brief Set parameter for Component and Scheduler.
	 */
	void ReconfigurableComponent::processAndForwardParameter(char *sType,char *sValue){
		
		if(this->controller != NULL){
			
			this->controller->setProperty(sType, sValue);
			
		}
		
	}
	
	/**
	 * \brief Implementation of ReconfigurableComponent::loadNewConfiguration()
	 */
	void ReconfigurableComponent::loadNewConfiguration(Configuration* newConfig, sc_time timeToLoad){
		this->setActivConfiguration(newConfig->getName());
		// wait time configurations needs to perform
		wait(timeToLoad);	
	}
	
}//namespace SystemC_VPC
