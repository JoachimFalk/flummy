#include <hscd_vpc_FCFSController.h>

namespace SystemC_VPC{
  
  /**
   * \brief Initializes instance of FCFSController
   */
  FCFSController::FCFSController(const char* name) : Controller(name){

    this->nextConfiguration = NULL;
    
  }

  /**
   * \brief Deletes instance of FCFSController
   */
  FCFSController::~FCFSController(){}
  
  /**
   * \brief Implementation of PreempetivController::addTasksToSchedule
   */
  void FCFSController::addTasksToSchedule(std::deque<p_struct* >& newTasks){
    this->waitInterval = NULL;

#ifdef VPC_DEBUG
        std::cerr << YELLOW("FCFSController "<< this->getName() <<"> addTasksToSchedule called! ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

    // first of all add tasks to local storage structure
    while(newTasks.size() > 0){
      this->readyTasks.push(newTasks.front());
      newTasks.pop_front();  
    }
    
    p_struct* currTask;
     
     bool currentConfigUsed = false;
     
    // now check which tasks to pass forward
    while(this->readyTasks.size()){
      currTask = this->readyTasks.front();

#ifdef VPC_DEBUG
      std::cerr << YELLOW("FCFSController "<< this->getName() <<"> processing task ") << currTask->name << endl;
#endif //VPC_DEBUG

      // get mapping for waiting task
      std::map<std::string, std::string >::iterator iter;
      iter = this->mapping_map_configs.find(currTask->name);
      // check if mapping exists
      if(iter != this->mapping_map_configs.end()){
          
          Configuration* reqConf = this->getManagedComponent()->getConfiguration(iter->second.c_str());
           
         if(reqConf == NULL){
            
            // managed component does not support mapped configuration
            std::cerr << YELLOW("FCFSController "<< this->getName() << "> Configuration not supported by component") << std::endl;
// FIX ME -->            // until now pop task
            this->readyTasks.pop();
            continue;
              
          }

          // check if current activ configuration fits required one
          if(reqConf == this->getManagedComponent()->getActivConfiguration()){

#ifdef VPC_DEBUG
          std::cerr << YELLOW("FCFSController "<< this->getName() <<"> current loaded configuration fits required one! ") << sc_simulation_time() << endl;
          std::cerr << YELLOW("FCFSController "<< this->getName() <<"> activ Configuration is activ? " << this->getManagedComponent()->getActivConfiguration()->isActiv()) << std::endl;
#endif //VPC_DEBUG
                
            this->tasksToProcess.push(currTask);
            this->runningTasks[currTask->pid] = currTask;
            
            // remove task that can be passed from waiting queue
          this->readyTasks.pop();
          
          currentConfigUsed = true;
          
          }else{  // check current configuration if or when it can be replaced

#ifdef VPC_DEBUG
          std::cerr << YELLOW("FCFSController "<< this->getName() <<"> currently num of running tasks = ") << this->runningTasks.size() << " at: " << sc_simulation_time() << endl;
#endif //VPC_DEBUG

          if(this->runningTasks.size() == 0 || reqConf == this->nextConfiguration){
            this->tasksToProcess.push(currTask);
            this->runningTasks[currTask->pid] = currTask;
              // remove task that can be passed from waiting queue
            this->readyTasks.pop();
            
              // load new configuration
                //this->nextConfigurations.push(reqConf);
                this->nextConfiguration = reqConf;
                
          }else{
            break;
          }
          }
/*
            sc_time* time = this->managedComponent->minTimeToIdle();


#ifdef VPC_DEBUG
          std::cerr << YELLOW("FCFSController "<< this->getName() <<"> current loaded configuration does not fit requiYELLOW one! ") << sc_simulation_time() << endl;
          std::cerr << YELLOW("FCFSController "<< this->getName() <<"> check current runtime of component=" << *time) << std::endl;
#endif //VPC_DEBUG

            // if current activ configuration is idle replace and execute task
            if(*time == SC_ZERO_TIME && currentConfigUsed == false && this->nextConfigurations.size() <= 0){

#ifdef VPC_DEBUG
            std::cerr << YELLOW("FCFSController "<< this->getName() <<"> able to load requested configuration: ") << reqConf->getName() << endl;
#endif //VPC_DEBUG
                this->tasksToProcess.push(currTask);
              // remove task that can be passed from waiting queue
            this->readyTasks.pop();
            
              // load new configuration
                this->nextConfigurations.push(reqConf);
                this->setLoadTime(this->managedComponent->getActivConfiguration(), reqConf, this->kill);
                
                //this->waitInterval = new sc_time(SC_ZERO_TIME);
                
              }else{
                // unable to process following waiting task so stop here
                // determine minimum time to wait until next request may be successful
                this->waitInterval = time;
                  
#ifdef VPC_DEBUG
            std::cerr << BLUE("FCFSController "<< this->getName() <<"> Cant delegate task right now!\n"
                << "Retrying in: " << this->waitInterval->to_default_time_units()) << std::endl;
#endif //VPC_DEBUG
                break;
              }
              
          delete time;

        }
        */
      }
    }
  }
   
  /*
   * \brief Implementation of FCFSController::getNextConfiguration
   */  
  Configuration* FCFSController::getNextConfiguration(){
    
    Configuration* next = this->nextConfiguration;
    this->nextConfiguration = NULL;
    return next;
  
  }
   
  /**
   * \brief Implementation of FCFSController::hasTaskToProcess()
   */
  bool FCFSController::hasTaskToProcess(){
  
     return (this->tasksToProcess.size() > 0);
  
  }
  
  /**
   * \brief Implementation of FCFSController::getNextTask()
   */
  p_struct* FCFSController::getNextTask(){
     
     p_struct* task;
     task = this->tasksToProcess.front();
     this->tasksToProcess.pop();
     return task;
   
   }
  
  /**
   * \brief Implementation of FCFSController::signalTaskEvent
   */
  void FCFSController::signalTaskEvent(p_struct* pcb){
  
#ifdef VPC_DEBUG
    std::cerr << "FCFSController " << this->getName() << "> got notified by task: " << pcb->name << std::endl;
#endif //VPC_DEBUG
    
    this->runningTasks.erase(pcb->pid);
    
    // if task has been killed and controlled instance is not killed solve decision here
    if(pcb->state == activation_state(aborted) && !this->getManagedComponent()->hasBeenKilled()){
      // recompute
      this->getManagedComponent()->compute(pcb);
    }else{
      this->getManagedComponent()->notifyParentController(pcb);
    }
        
#ifdef VPC_DEBUG
    if(pcb->state == activation_state(aborted)){
      std::cerr << YELLOW("FCFSController> task: " << pcb->name << " got killed!")  << std::endl;
    }
#endif //VPC_DEBUG
      
    // if there are no running task and still ready its time to wakeUp ReconfigurableComponent
    if(this->runningTasks.size() == 0 && this->readyTasks.size() != 0){

#ifdef VPC_DEBUG
      std::cerr << "FCFSController> waking up component thread!" << std::endl;
#endif //VPC_DEBUG

      this->getManagedComponent()->wakeUp();
    }
  }
  
} //namespace SystemC_VPC
