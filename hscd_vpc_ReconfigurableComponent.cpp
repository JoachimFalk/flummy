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
	ReconfigurableComponent::ReconfigurableComponent(sc_module_name name, AbstractController* controller): sc_module(name) {
		
		SC_THREAD(schedule_thread);
		strcpy(this->componentName, name);
		this->setController(controller);
		
#ifndef NO_VCD_TRACES
		std::string tracefilename = this->componentName;
		char tracefilechar[VPC_MAX_STRING_LENGTH];
		char* traceprefix= getenv("VPCTRACEFILEPREFIX");
		if(NULL != traceprefix){
			tracefilename.insert(0,traceprefix);
		}
		strcpy(tracefilechar,tracefilename.c_str());
		this->traceFile = sc_create_vcd_trace_file(tracefilechar);
		((vcd_trace_file*)this->traceFile)->sc_set_vcd_time_unit(-9);	      
#endif //NO_VCD_TRACES
		
		if(!this->isActiv()){
#ifdef VPC_DEBUG
			std::cerr << RED(this->getName() << "> Activating") << std::endl;
#endif //VPC_DEBUG
			this->setActiv(true);
		}
		
	}
	
	/**
	 * \brief Implementation of destructor of ReconfigurableComponent
	 */
	ReconfigurableComponent::~ReconfigurableComponent(){
	
		delete this->controller;
		
		std::map<std::string, Configuration*>::iterator iter;
		for(iter = this->config_map_by_name.begin();
			iter != this->config_map_by_name.end(); iter++){
		
			delete iter->second;
		}
		
		this->config_map_by_name.clear();
	}
	
	/**
	 * \brief Implementation of ReconfigurableComponent::schedule_thread()
	 */
	void ReconfigurableComponent::schedule_thread(){
		/**
		 ** set up configurations traces
		 **/
		
		// set all traced configurations to passiv
		std::map<std::string, sc_signal<trace_value>* >::iterator iter;
		for(iter = this->trace_map_by_name.begin(); iter != this->trace_map_by_name.end(); iter++){
			*(iter->second) = S_PASSIV;
		}
		// set loaded config activ
		if(this->activConfiguration != NULL){
			iter = this->trace_map_by_name.find(this->activConfiguration->getName());
			if(iter != this->trace_map_by_name.end()){
				*(iter->second) = S_ACTIV;
			}
		}
		
		/*******************************************
		 ** 
		 ** while(true){
		 ** 			wait for new tasks;
		 ** 			pass Tasks to Controller;
		 ** 			request delegatable tasks from Controller;
		 ** 			delegate them by using informations from Controller;
		 **				if new configuration requested?
		 **					load configuration
		 **	}
		 **
		 *******************************************/
		 
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

					wait(this->notify_schedule_thread | this->notify_preempt);
				
				}else{
				
#ifdef VPC_DEBUG
					std::cerr << RED("ReconfigurableComponent "<< this->getName() <<"> waiting for redelegation in: ") << minTimeToWait->to_default_time_units() << endl;
#endif //VPC_DEBUG
	
					wait(*minTimeToWait, this->notify_schedule_thread | this->notify_preempt);
					delete minTimeToWait;
					minTimeToWait = NULL;
					
				}
			}else{
				
				newTasksDuringLoad = false;
				
			}	
			
			
#ifdef VPC_DEBUG
			std::cerr << RED("ReconfigurableComponent "<< this->getName() <<"> got notified at time: " << sc_simulation_time() 
						<< " with num of new tasks= " << this->newTasks.size()) << endl;
#endif //VPC_DEBUG

			// if component is preempted wait till resume
			if( !this->isActiv() ){

#ifdef VPC_DEBUG
				std::cerr << RED("ReconfigurableComponent "<< this->getName() <<"> not activ going to sleep at time: ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG
				
				this->controller->signalPreemption();
				this->storeActivConfiguration(this->killed);
				if(this->killed){
					this->activConfiguration = NULL;
				}
				
				wait(this->notify_resume);
				
				this->loadConfiguration(this->activConfiguration);
				this->controller->signalResume();
				
#ifdef VPC_DEBUG
				std::cerr << RED("ReconfigurableComponent "<< this->getName() <<"> awoke at time: ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

			}

			// inform controller about new tasks
			this->controller->addTasksToSchedule(this->newTasks);

#ifdef VPC_DEBUG
				std::cerr << RED("ReconfigurableComponent "<< this->getName() <<"> finished delegation to controller !") << sc_simulation_time() << endl;
#endif //VPC_DEBUG
			
			// delegate all processable tasks
			// points to currently viewed task to delegate
			p_struct* currTask;
			// points to component to delegate task to
			AbstractComponent* currComp;
			
			while(this->controller->hasTaskToProcess()){

				currTask = this->controller->getNextTask();
				
#ifdef VPC_DEBUG
				std::cerr << RED("ReconfigurableComponent "<< this->getName() <<"> still task to forward: ") << currTask->name << " at "  << sc_simulation_time() << endl;
#endif //VPC_DEBUG


				currComp = this->controller->getMappedComponent(currTask);
				currComp->compute(currTask);
			
			}


#ifdef VPC_DEBUG
				std::cerr << RED("ReconfigurableComponent "<< this->getName() <<"> finished forwarding and checking config !") << sc_simulation_time() << endl;
#endif //VPC_DEBUG
			
			// check if new configuration has to be loaded
			// points to required configuration for a given task
			Configuration* nextConfig;
			nextConfig = this->controller->getNextConfiguration();
			// there is new configuration load it
			if(nextConfig != NULL && nextConfig != this->activConfiguration){

#ifdef VPC_DEBUG
				std::cerr << BLUE("ReconfigurableComponent " << this->getName() << "> new config to load: ") << nextConfig->getName() << std::endl;
#endif //VPC_DEBUG
			
				// perform reconfiguration
				if(this->storeActivConfiguration(this->controller->preemptByKill())){
					if(!this->loadConfiguration(nextConfig)){
					
#ifdef VPC_DEBUG
						std::cerr << BLUE("ReconfigurableComponent " << this->getName() << "> failed loading!") << std::endl;
#endif //VPC_DEBUG
					
					}

#ifdef VPC_DEBUG
				std::cerr << BLUE("ReconfigurableComponent " << this->getName() << "> new configuration loaded: " << this->activConfiguration->getName()) << std::endl;
#endif //VPC_DEBUG

				}else{
				
#ifdef VPC_DEBUG
					std::cerr << BLUE("ReconfigurableComponent " << this->getName() << "> failed storing!") << std::endl;
#endif //VPC_DEBUG
				
				}

				//check if new task register during loadphase which may have caused wait
				if(this->newTasks.size() > 0){

#ifdef VPC_DEBUG
				std::cerr << RED("New tasks during configuration load!") << std::endl;
#endif //VPC_DEBUG

					newTasksDuringLoad = true;
				}
			}
			
    		// check if controller request special interval to be called next time
    		minTimeToWait = this->controller->getWaitInterval();
    		
		}
	}
	
    /**
	 * \brief Implementation of ReconfigurableComponent::addConfiguration
	 */
	void ReconfigurableComponent::addConfiguration(const char* name, Configuration* config){
		
		assert(name != NULL);
		assert(config != NULL);
		
#ifndef NO_VCD_TRACES
    	sc_signal<trace_value>* newsignal = new sc_signal<trace_value>();
    	trace_map_by_name.insert(pair<string,sc_signal<trace_value>*>(name, newsignal));
    	sc_trace(this->traceFile, *newsignal, name);
#endif //NO_VCD_TRACES

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
		if(newConfig != this->config_map_by_name.end()){
			
			this->activConfiguration = newConfig->second;
			
			// activate new configration
			this->activConfiguration->resume();
			
#ifdef VPC_DEBUG
			std::cerr << "ReconfigurableComponent> activ Configuration " << this->activConfiguration->getName() << " is activ "
					  << this->activConfiguration->isActiv() << std::endl;
#endif //VPC_DEBUG

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
	void ReconfigurableComponent::setController(AbstractController* controller){
		
		assert(controller != NULL);
		
		this->controller = controller;
		// register backward to controller
		this->controller->setManagedComponent(this);
	} 

	/**
	 * \brief Implementation of ReconfigurableComponent::preempt
	 */
	void ReconfigurableComponent::preempt(bool kill){
	    
		// only preempt activ component
		if(this->isActiv()){
			this->killed = kill;
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
			this->killed = false;
			this->setActiv(true);
			this->notify_resume.notify();	
		}
		
	}

	/**
	 * \brief Implementation of ReconfigurableComponent::timeToPreempt
	 */	
	sc_time* ReconfigurableComponent::timeToPreempt(){
		sc_time* time = NULL;
		
		// only if component is activ we have time for preemption
		if(this->isActiv() && this->activConfiguration != NULL){
			
			time = this->activConfiguration->timeToPreempt();
			*time += this->activConfiguration->getStoreTime();
			
		}else{
			
			time = new sc_time(SC_ZERO_TIME);
		
		}
		
		return time;
	}
		
	/**
	 * \brief Implementation of ReconfigurableComponent::timeToResume
	 */	
	sc_time* ReconfigurableComponent::timeToResume(){
		
		sc_time* time = NULL;
		
		// only if component is inactiv we have time to resume
		if(!this->isActiv() && this->activConfiguration != NULL){
			
			time = this->activConfiguration->timeToResume();
			*time += this->activConfiguration->getLoadTime();
			
		}else{
			
			time = new sc_time(SC_ZERO_TIME);
		
		}
		
		return time;
	
	}
		
	/**
	 * \brief An implementation of AbstractComponent::compute(const char *, const char *, VPC_Event).
	 */
	void ReconfigurableComponent::compute( const char* name, const char* funcname, VPC_Event* end){
		
		// send compute request to controller
		p_struct* pcb = Director::getInstance().getProcessControlBlock(name);
		pcb->blockEvent = end;
		this->compute(pcb);
		
	}
	    
	/**
	 * \brief An implementation of AbstractComponent::compute(const char *, VPC_Event).
	 */
	void ReconfigurableComponent::compute( const char *name, VPC_Event *end){
		
		this->compute(name, "", end);
		
	}
	
	/**
	 * \brief An implementation of AbstractComponent::compute(p_struct*, const char *).
	 */
	void ReconfigurableComponent::compute(p_struct* pcb){
		
		this->newTasks.push_back(pcb);
		
		this->notify_schedule_thread.notify();
		
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
	
	bool ReconfigurableComponent::storeActivConfiguration(bool kill){
						
		//remember start time of storing
		sc_time storeStart = sc_time_stamp();
		sc_time timeToStore = SC_ZERO_TIME;
			
		// stop currently running components
		if(this->activConfiguration != NULL){

#ifdef VPC_DEBUG
			std::cerr << "ReconfigurableComponent" << this->getName() << "> trying to store config kill=" << kill << std::endl;
#endif //VPC_DEBUG

			//update time for reconfiguration if storing is required
			if(!kill){
				sc_time* time = this->activConfiguration->timeToPreempt();
				timeToStore += *time;
				delete time;
			}
			
			this->activConfiguration->preempt(kill);

#ifndef NO_VCD_TRACES
			if(1==trace_map_by_name.count(this->activConfiguration->getName())){
				std::map<std::string, sc_signal<trace_value>* >::iterator iter = trace_map_by_name.find(this->activConfiguration->getName());
				*(iter->second) = S_PASSIV;
			}
#endif //NO_VCD_TRACES

			//check if preemption happend not here !!!
			/*
			if(this->reconfigurationInterrupted(storeStart, timeToStore)){
				return false;
			}
			*/
					
			// Simulate store time if required
			if(!kill){
				//update time for reconfiguration
				timeToStore += this->activConfiguration->getStoreTime();
			}
			
			wait(timeToStore, this->notify_preempt);
					
			//check if preemption happend
			if(this->reconfigurationInterrupted(storeStart, timeToStore)){

#ifdef VPC_DEBUG
				std::cerr << "ReconfigurableComponent " << this->getName() << "> storing configuration has been interrupted " << std::endl;
#endif //VPC_DEBUG
						
				this->activConfiguration->setStored(false);
				return false;
						
			}		
					
			this->activConfiguration->setStored(true);
										
		}
		
		return true;
	}
	
	bool ReconfigurableComponent::loadConfiguration(Configuration* config){
						
		//remember start time of storing
		sc_time loadStart = sc_time_stamp();
		sc_time timeToLoad = SC_ZERO_TIME;
				
		//load new configuration
		if(config != NULL){
			
#ifdef VPC_DEBUG
			std::cerr << "ReconfigurableComponent " << this->getName() << "> loading configuration config= " << config->getName() << std::endl;
#endif //VPC_DEBUG

			//simulate loading time
			timeToLoad += config->getLoadTime();
			wait(config->getLoadTime(), this->notify_preempt);
			
			// check if preemption happened
			if(reconfigurationInterrupted(loadStart, timeToLoad)){
				this->activConfiguration = NULL;
				return false;
			}
					
			this->activConfiguration = config;
				
			sc_time* time = this->activConfiguration->timeToResume();
			timeToLoad += *time;
					
			this->activConfiguration->resume(); 
			// wait time of resume
			wait(*time, this->notify_preempt);
			delete time;		
			
			if(reconfigurationInterrupted(loadStart, timeToLoad)){
				return false;
			}
			
#ifndef NO_VCD_TRACES
			if(1==trace_map_by_name.count(config->getName())){
				std::map<std::string, sc_signal<trace_value>* >::iterator iter = trace_map_by_name.find(config->getName());
				*(iter->second) = S_ACTIV;
			}
#endif //NO_VCD_TRACES
			
		}	
		
		return true;
	}
	
	bool ReconfigurableComponent::reconfigurationInterrupted(sc_time timeStamp, sc_time interval){
		
		//check if preemption happend
		sc_time elapsedTime = sc_time_stamp() - timeStamp;
		
		if(elapsedTime.value() < interval.value()){

#ifdef VPC_DEBUG
			std::cerr << "ReconfigurableComponent " << this->getName() << "> reconfiguration phase has been interrupted " << std::endl;
			std::cerr << "ReconfigurableComponent " << this->getName() << "> startTime= " << timeStamp << " interval= " << interval
			<< " elapsedTime= " << elapsedTime << std::endl;
#endif //VPC_DEBUG

			return true;
			
		}
		
		return false;
	}
		
}//namespace SystemC_VPC
