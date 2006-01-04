#include <hscd_vpc_NonPreemptivController.h>

namespace SystemC_VPC{

	NonPreemptivController::NonPreemptivController(const char* name){

		strcpy(this->controllerName, name);
		
	}

	NonPreemptivController::~NonPreemptivController(){}
	
	/**
	 * \brief Implementation of NonPreemptivController::delegateTask
	 */
	void NonPreemptivController::compute(const char* name, const char* funcname, CoSupport::SystemC::Event* end){
/*	
#ifdef VPC_DEBUG
		std::cerr << GREEN("NonPreemptivController "<< this->getName() <<"> compute(") << name << GREEN(", ") << funcname 
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
		std::cerr << GREEN("NonPreemptivController "<< this->getName() <<"> SIMULATE SYNCHRON CALL") << std::endl;
#endif //VPC_DEBUG
			p_struct* p = Director::getInstance().getProcessControlBlock(name);
			p->blockEvent = currTask->end;
			CoSupport::SystemC::wait(*(p->blockEvent));
			delete p->blockEvent;
			p->blockEvent = NULL;
#ifdef VPC_DEBUG
		std::cerr << GREEN("NonPreemptivController "<< this->getName() <<"> SIMULATED SYNCHRON CALL") << std::endl;
#endif //VPC_DEBUG
		}
		return;
		*/
	}
   
	/**
	 * \brief Implementation of NonPreemptivController::delegateTask
	 */
	void NonPreemptivController::compute(const char *name, CoSupport::SystemC::Event *end){
	
		this->compute(name, "", end);
	
	}

	
	/**
	 * \brief Implementation of NonPreemptivController::registerComponent
	*/
	void NonPreemptivController::registerComponent(AbstractComponent* comp){
		// do nothing right now
	}
	    
	/**
	 * \brief Implementation of NonPreemptivController::registerMapping
	 */
	void NonPreemptivController::registerMapping(const char* taskName, const char* compName){
		
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
	 * \brief Implementation of  NonPreemptivController::setProperty
	 */
	void NonPreemptivController::setProperty(char* key, char* value){
	}
	
	/**
	 * \brief Implementation of PreempetivController::addTasksToSchedule
	 */
	void NonPreemptivController::addTasksToSchedule(std::deque<std::pair<p_struct* , const char* > >& newTasks){
		this->waitInterval = NULL;

		// first of all add tasks to local storage structure
		while(newTasks.size() > 0){
			this->waitingTasks.push(newTasks.front());
			newTasks.pop_front();	
		}
		
		std::pair<p_struct*, const char*> currTask;
	 	
		// now check which tasks to pass forward
		while(this->waitingTasks.size()){
			currTask = this->waitingTasks.front();
			
			// get mapping for waiting task
			std::map<std::string, std::string >::iterator iter;
			iter = this->mapping_map_configs.find((currTask.first)->name);
			// check if mapping exists
			if(iter != this->mapping_map_configs.end()){
					
	  			Configuration* reqConf = this->managedComponent->getConfiguration(iter->second.c_str());
	 				
	 			if(reqConf == NULL){
		    		
		    		// managed component does not support mapped configuration
		    		std::cerr << RED("NonPreemptivController "<< this->getName() << "> Configuration not supported by component") << std::endl;
// FIX ME -->		    		// until now pop task
		    		this->waitingTasks.pop();
		    		continue;
		  				
	  			}

	  			// check if current activ configuration fits required one
	  			if(reqConf == this->managedComponent->getActivConfiguration()){

#ifdef VPC_DEBUG
				std::cerr << RED("NonPreemptivController "<< this->getName() <<"> current loaded configuration fits required one! ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG
	  	    			
	    			this->tasksToProcess.push(currTask);
	    			// remove task that can be passed from waiting queue
					this->waitingTasks.pop();
				
	    		}else{  // check current configuration if or when it can be replaced
	  				sc_time* time = this->managedComponent->minTimeToIdle();


#ifdef VPC_DEBUG
				std::cerr << RED("NonPreemptivController "<< this->getName() <<"> current loaded configuration does not fit required one! ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

#ifdef VPC_DEBUG
					std::cerr << YELLOW("NonPreemptivController "<< this->getName() <<"> check current runtime of component=" << *time) << std::endl;
#endif //VPC_DEBUG

	  				// if current activ configuration is idle replace and execute task
	  				if(*time == SC_ZERO_TIME){

#ifdef VPC_DEBUG
				std::cerr << RED("NonPreemptivController "<< this->getName() <<"> able to load requested configuration: ") << reqConf->getName() << endl;
#endif //VPC_DEBUG
	      			
	  					// load new configuration
	      				this->nextConfigurations.push(reqConf);
	      				this->setLoadTime(this->managedComponent->getActivConfiguration(), reqConf, false);
	      				
	      				std::cerr << YELLOW(this->loadTime) << std::endl;
	      				this->waitInterval = new sc_time(this->loadTime);
	      				
	      			}else{
	      				// unable to process following waiting task so stop here
	      				// determine minimum time to wait until next request may be successful
	      				this->waitInterval = time;
	      					
#ifdef VPC_DEBUG
						std::cerr << BLUE("NonPreemptivController "<< this->getName() <<"> Cant delegate task right now!\n"
								<< "Retrying in: " << this->waitInterval->to_default_time_units()) << std::endl;
#endif //VPC_DEBUG
	      				break;
	      			}
	      			
					delete time;
					break;
				}
				
			}
		}
	}
	 
	/*
	 * \brief Implementation of NonPreemptivController::getNextConfiguration
	 */	
  	Configuration* NonPreemptivController::getNextConfiguration(){
	 	
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
	void NonPreemptivController::setLoadTime(Configuration* oldConfig, Configuration* newConfig, bool killOld){
		this->loadTime = this->loadTime_map[newConfig->getName()];
		
		// check if old config has to be stored and is not null
		if(!killOld && oldConfig != NULL){
		
			this->loadTime += this->storeTime_map[oldConfig->getName()];
	 	
		}
	}
	 
	/**
	 * \brief Implementation of NonPreemptivController::getMappedComponent()
	 */  
	AbstractComponent* NonPreemptivController::getMappedComponent(p_struct* task){
		
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
	bool NonPreemptivController::hasTaskToProcess(){
	
	 	return (this->tasksToProcess.size() > 0);
	
	}
	
	/**
	 * \brief Implementation of NonPreemptivController::getNextTask()
	 */
	std::pair<p_struct*, const char* > NonPreemptivController::getNextTask(){
	 	
	 	std::pair<p_struct*, const char* > task;
	 	task = this->tasksToProcess.front();
	 	this->tasksToProcess.pop();
	 	return task;
	 
	 }
	
} //namespace SystemC_VPC
