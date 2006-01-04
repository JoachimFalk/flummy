#include <hscd_vpc_PreemptivController.h>

namespace SystemC_VPC{

	PreemptivController::PreemptivController(const char* name){
		
		strcpy(this->controllerName, name);
    	
  	}

  	PreemptivController::~PreemptivController(){}
	
	/**
	 * \brief Implementation of PreemptivController::delegateTask
	 */
	void PreemptivController::compute(const char* name, const char* funcname, CoSupport::SystemC::Event* end){
	/*
#ifdef VPC_DEBUG
		std::cerr << GREEN("PreemptivController> compute(") << name << GREEN(", ") << funcname 
				<< GREEN(") at: ") << sc_simulation_time() << std::endl;
#endif //VPC_DEBUG

		// reset reconfiguration time to NULL as not happend up to know
		this->loadTime = NULL;

		my_taskInfo* currTask = new my_taskInfo();
		currTask->name = name;
		currTask->funcname = funcname;

		if(end != NULL){
  			currTask->end = end;
		}else{
			currTask->end = new CoSupport::SystemC::Event();
		}
		
		// add task to waiting tasks
		this->waiting_tasks.push(currTask);
		this->notify_control_thread.notify();

		// use blocking mode if synchron call appeared
		if( end == NULL ){

#ifdef VPC_DEBUG
			std::cerr << GREEN("PreemptivController> SIMULATE SYNCHRON CALL") << std::endl;
#endif //VPC_DEBUG

		p_struct* p = Director::getInstance().getProcessControlBlock(name);
		p->blockEvent = currTask->end;
		CoSupport::SystemC::wait(*(p->blockEvent));
		delete p->blockEvent;
		p->blockEvent = NULL;

#ifdef VPC_DEBUG
		std::cerr << GREEN("PreemptivController> SIMULATED SYNCHRON CALL") << std::endl;
#endif //VPC_DEBUG
		}
    
		return;
		*/
	}
   
	/**
	 * \brief Implementation of PreemptivController::delegateTask
	 */
	void PreemptivController::compute(const char *name, CoSupport::SystemC::Event *end){
		
		this->compute(name, "", end);
	
	}

	/**
	 * \Implementation of PreemptivController::control_thread
	 */
	 /*
	void PreemptivController::control_thread(){
		// time which PreemptivController has to wait at least to perform reconfiguration
		sc_time* minTimeToWait = NULL;
		// points to currently viewed task to delegate
		my_taskInfo* currTask;

		while(1){
							
			// check if PreemptivController has to wait for configuration to "run to completion"
			if(minTimeToWait == NULL){
				
#ifdef VPC_DEBUG
				std::cerr << RED("PreemptivController "<< this->getName() <<"> waiting for new tasks at ")  << sc_simulation_time() << endl;
#endif //VPC_DEBUG

				wait(notify_control_thread | notify_preempt);
      		}else{
				
#ifdef VPC_DEBUG
				std::cerr << RED("PreemptivController "<< this->getName() <<"> waiting for redelegation at ")
						<< sc_simulation_time() << RED(" wake up in ") << minTimeToWait->to_default_time_units() << endl;
#endif //VPC_DEBUG

				this->wait(*minTimeToWait, notify_control_thread | notify_preempt);
				delete minTimeToWait;
				minTimeToWait = NULL;
			}
			
			
#ifdef VPC_DEBUG
      		std::cerr << RED("PreemptivController "<< this->getName() <<"> got notified at time: ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

			// if component is preempted wait till resume
			if(! this->isActiv()){

#ifdef VPC_DEBUG
				std::cerr << RED("PreemptivController "<< this->getName() <<"> not activ going to sleep at time: ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

				this->wait(notify_resume);

#ifdef VPC_DEBUG
				std::cerr << RED("PreemptivController "<< this->getName() <<"> actived going at time: ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

			}

			// check waiting tasks, if they can be delegate to components right now
			while(this->waiting_tasks.size() != 0){

				// reuse currTask to point to first element of waiting queue
				currTask = this->waiting_tasks.front();
	
	#ifdef VPC_DEBUG
				std::cerr << RED("PreemptivController "<< this->getName() <<"> processing task at time: ") << sc_simulation_time() << endl;
				std::cerr << RED("PreemptivController "<< this->getName() <<"> task=") << WHITE(currTask->name) << RED(" funcname=") << currTask->funcname << std::endl;
	#endif //VPC_DEBUG
	  
				// get mapping for waiting task
				std::map<std::string, std::string >::iterator iter;
				iter = this->mapping_map_configs.find(currTask->name);
				// check if mapping exists
				if(iter != this->mapping_map_configs.end()){
									
					Configuration* reqConf = this->managedComponent->getConfiguration(iter->second.c_str());
					 				
					if(reqConf == NULL){
		    		
						// managed component does not support mapped configuration
						std::cerr << RED("ERROR: PreemptivController " << this->getName() << "> Configuration not supported by component") << std::endl;
						// until now pop task
						this->waiting_tasks.pop();
						continue;
							  				
					}

#ifdef VPC_DEBUG
				std::cerr << RED("PreemptivController "<< this->getName() <<"> requested configuration: ") << reqConf->getName() << endl;
#endif //VPC_DEBUG
	
					// check if current activ configuration fits required one
					if(reqConf == this->managedComponent->getActivConfiguration()){
						// delegate task to resource
						this->passTaskToResource(currTask->name, currTask->funcname, currTask->end); 
					}else{  // check current configuration if or when it can be replaced
						// currently replace config if necessary
	
#ifdef VPC_DEBUG
						std::cerr << YELLOW("PreemptivController "<< this->getName() <<"> preempt current config and (re-)start requested one!") << std::endl;
#endif //VPC_DEBUG
	
						// suspend activ configuration
						Configuration* activConfig = this->managedComponent->getActivConfiguration();
						if(activConfig != NULL){
							
							activConfig->preemptComponents();
							this->preemptedConfs.push_back(activConfig);
#ifdef VPC_DEBUG
							std::cerr << YELLOW("PreemptivController "<< this->getName() <<"> preempted activ configuration!") << std::endl;
							std::cerr << YELLOW("PreemptivController "<< this->getName() 
								<< ">  size of preempted configurations: ") << this->preemptedConfs.size() << std::endl;
#endif //VPC_DEBUG

						}
						
						// load new configuration
						this->loadNewConfiguration(reqConf);
						
						this->needToReactivateConfiguration(reqConf);
										
#ifdef VPC_DEBUG
							std::cerr << YELLOW("PreemptivController "<< this->getName() <<"> resumed configuration!") << std::endl;
							std::cerr << YELLOW("PreemptivController "<< this->getName() 
								<< ">  size of preempted configurations: ") << this->preemptedConfs.size() << std::endl;
#endif //VPC_DEBUG
  				
						// delegate task to resource
						this->passTaskToResource(currTask->name, currTask->funcname, currTask->end);
					}
				
					// first element successful delegated so remove it
					this->waiting_tasks.pop();
					delete currTask;
						
					// we have reconfigured and passed task
					// lets propagate this fast and wait one cycle
					// this is needed if new task entered requesting other configuration
					wait(SC_ZERO_TIME);
				
				}else{
					// unable to process following waiting task so stop here
					// delete task as we cannot map it ?!? 
					// callback to upper layer somehow ?!?
					break;
				}
			}
			
			// check if there are still suspended configurations
			
			sc_time* time = this->managedComponent->minTimeToIdle();
			
			if(*time == SC_ZERO_TIME){
				Configuration* next = NULL;
				
				while(this->preemptedConfs.size() > 0
						&&	*time == SC_ZERO_TIME){
					
					next = this->preemptedConfs.front();
					this->preemptedConfs.pop_front();
					
					assert(next != NULL);
					
					delete time;
					time = next->minTimeToIdle();			
				}
				
				// check if configuration found with runtime > 0
				if(next != NULL && *time != SC_ZERO_TIME){
					// found next configuration to execute
					this->loadNewConfiguration(next);
					next->resumeComponents();
					minTimeToWait = time;
				}else{
					delete time;
				}
			}else{
				delete time;
			}
						
			wait(SC_ZERO_TIME);
		}
  	}
	*/

	/**
	 * \brief Implementation of PreemptivController::registerComponent
	 */
	void PreemptivController::registerComponent(AbstractComponent* comp){
			// do nothing right now
	}
		    
	/**
	 * \brief Implementation of PreemptivController::registerMapping
	 */
	void PreemptivController::registerMapping(const char* taskName, const char* compName){
		
		// register direct mapping to component
		this->mapping_map_component_ids[taskName] = compName;
			
		// register mapping to configuration
		std::map<std::string, Configuration* > configs;
		configs = this->managedComponent->getConfigurations();
		std::map<std::string, Configuration* >::iterator iter;

		// go through all configurations and find right containing component
		for(iter = configs.begin(); iter != configs.end(); iter++){
			AbstractComponent* comp = (iter->second)->getComponent(compName);
			if(comp != NULL){
				// found component
				this->mapping_map_configs.insert(std::pair<std::string, std::string>(taskName, (iter->second)->getName()));
				// exactly one match possible
				break;
      		}	
    	}
	}
	
	/**
	 * \brief Implementation of PreemptivController::isConfigurationPreempted
	 */
	bool PreemptivController::needToReactivateConfiguration(Configuration* config){

#ifdef VPC_DEBUG
		std::cerr << YELLOW("PreemptivController "<< this->getName() <<"> check if reactivation needed") << std::endl;
#endif // VPC_DEBUG

		//check if requested configuration is also in preemptedList
		std::list<Configuration* >::iterator iter = this->preemptedConfs.begin();
		for(; iter != this->preemptedConfs.end(); iter++){
			if(*iter == config){
				this->preemptedConfs.erase(iter);
#ifdef VPC_DEBUG
				std::cerr << GREEN("PreemptivController "<< this->getName() <<"> Initiate Activation for Configuration: "
						<< config->getName() ) << std::endl;
#endif //VPC_DEBUG
				config->resumeComponents();
				return true;
			}
		}
		
		return false;
	}
	
	/**
	 * \brief Implementation of  PreemptivController::setProperty
	 */
	void PreemptivController::setProperty(char* key, char* value){
		
		if(0==strncmp(key,"timeslice",strlen("timeslice"))){
#ifdef VPC_DEBUG
      		std::cerr << YELLOW("PreemptivController " << this->getName()
      				<< " > received Property: " << key << " with value "
      				<< value ) << std::endl;
#endif //VPC_DEBUG
		}
    }
	
	/**
	 * \brief Implementation of PreempetivController::addTasksToSchedule
	 */
	 void PreemptivController::addTasksToSchedule(std::deque<std::pair<p_struct* , const char* > >& newTasks){
	 }

	/*
	 * \brief Implementation of NonPreemptivController::getNextConfiguration
	 */	
  	Configuration* PreemptivController::getNextConfiguration(){
	 	
		Configuration* nextConfiguration = NULL;
	 	
		if(this->nextConfigurations.size() > 0){
	 		
			nextConfiguration = this->nextConfigurations.front();
			this->nextConfigurations.pop();
			
		}
		
		return nextConfiguration;
	}
	
	/**
	 * \brief Implementation of NonPreemptivController::setLoadTime()
	 */
	void PreemptivController::setLoadTime(Configuration* oldConfig, Configuration* newConfig, bool killOld){
		this->loadTime = this->loadTime_map[newConfig->getName()];
		
		if(!killOld){
		
			this->loadTime += this->storeTime_map[oldConfig->getName()];
	 	
		}
	}
	 
	/**
	 * \brief Implementation of NonPreemptivController::getMappedComponent()
	 */  
	AbstractComponent* PreemptivController::getMappedComponent(p_struct* task){
		
		// determine required configuration
		std::string configName = this->mapping_map_configs[task->name];
		Configuration* conf = this->managedComponent->getConfiguration(configName.c_str());
		// get mapped component from configuration
		AbstractComponent* comp = conf->getComponent((this->mapping_map_component_ids[task->name]).c_str());
		
		return comp;
	}
	
	/**
	 * \brief Implementation of NonPreemptivController::hasTaskToProcess()
	 */
	bool PreemptivController::hasTaskToProcess(){
	
	 	return (this->tasksToProcess.size() > 0);
	
	}
	
	/**
	 * \brief Implementation of NonPreemptivController::getNextTask()
	 */
	std::pair<p_struct*, const char* > PreemptivController::getNextTask(){
	 	
	 	std::pair<p_struct*, const char* > task;
	 	task = this->tasksToProcess.front();
	 	this->tasksToProcess.pop();
	 	return task;
	 
	 }

} //namespace SystemC_VPC

