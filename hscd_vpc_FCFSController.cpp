#include <hscd_vpc_FCFSController.h>

namespace SystemC_VPC{
  
  /**
   * \brief Initializes instance of FCFSController
   */
  FCFSController::FCFSController(AbstractController* controller) : ConfigurationScheduler(controller){

    //this->nextConfiguration = 0;
    
  }

  /**
   * \brief Deletes instance of FCFSController
   */
  FCFSController::~FCFSController(){
    assert(this->readyTasks.size() == 0);
    assert(this->runningTasks.size() == 0);
    assert(this->tasksToProcess.size() == 0);
  }
  
  /**
   * \brief Implementation of PreempetivController::addTasksToSchedule
   */
  void FCFSController::addTaskToSchedule(ProcessControlBlock* newTask, unsigned int config){
    this->waitInterval = NULL;

#ifdef VPC_DEBUG
        std::cerr << YELLOW("FCFSController "<< this->getController().getName() <<"> addTasksToSchedule called! ") 
          << "For task " << newTask->getName() << " with required configuration id " << config << " at " << sc_simulation_time() << endl;
        std::cerr << YELLOW("FCFSController "<< this->getController().getName() <<"> addTasksToSchedule called! ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

    // first of all add task to local storage structure
    std::pair<ProcessControlBlock*, unsigned int> entry(newTask, config);
    this->readyTasks.push_back(entry);
  }

  /**
   * \brief Implementation of FCFSController::performSchedule
   */
  void FCFSController::performSchedule(){
    
    ProcessControlBlock* currTask;
    unsigned int reqConfig = 0;
    // now check which tasks to pass forward
    while(this->readyTasks.size()){
      currTask = this->readyTasks.front().first;
      reqConfig = this->readyTasks.front().second;
      
#ifdef VPC_DEBUG
      std::cerr << YELLOW("FCFSController "<< this->getController().getName() <<"> processing task ") << currTask->getName()
          << " with required configuration id " << reqConfig << endl;
      if(this->getManagedComponent()->getActivConfiguration() == NULL){
        std::cerr << YELLOW("FCFSController "<< this->getController().getName() <<"> no activ configuration for managed component ") << endl;
      }else{
        std::cerr << YELLOW("FCFSController "<< this->getController().getName() <<"> activ configuration for managed component is ") 
          << this->getManagedComponent()->getActivConfiguration()->getName() << " with id " 
          << this->getManagedComponent()->getActivConfiguration()->getID() << endl;
        std::cerr << YELLOW("FCFSController "<< this->getController().getName() <<"> currently running num of tasks on conf ") << this->runningTasks.size() << endl;
      }
      std::cerr << YELLOW("FCFSController "<< this->getController().getName() <<"> next configuration is set to ") << this->nextConfiguration << endl;
#endif //VPC_DEBUG
          
      // check if current activ configuration fits required one and no new configuration comes next
      /*
      if(this->getManagedComponent()->getActivConfiguration() != NULL 
          && config == this->getManagedComponent()->getActivConfiguration()->getID()
          && this->nextConfiguration == 0){

#ifdef VPC_DEBUG
          std::cerr << YELLOW("FCFSController "<< this->getController().getName() <<"> current loaded configuration fits required one! ") 
            << sc_simulation_time() << endl;
          if(this->getManagedComponent()->getActivConfiguration() != NULL){
            std::cerr << YELLOW("FCFSController "<< this->getController().getName() <<"> activ Configuration is activ? " << this->getManagedComponent()->getActivConfiguration()->isActiv()) << std::endl;
          }
#endif //VPC_DEBUG

          this->tasksToProcess.push(currTask);
          this->runningTasks[currTask->getPID()] = currTask;

          // remove task that can be passed from waiting queue
          this->readyTasks.pop_front();
 
        }else */ // check current configuration if it can be replaced
        // check if 
        if((this->getManagedComponent()->getActivConfiguration() == NULL && this->nextConfiguration == 0) // no activ and no next conf selected
            || (this->runningTasks.size() == 0 && this->nextConfiguration == 0) // or no running tasks and no next conf selected
            || reqConfig == this->nextConfiguration){ // or required conf fits already selected one

#ifdef VPC_DEBUG
          std::cerr << YELLOW("FCFSController "<< this->getController().getName() <<"> can process task ") << currTask->getName() 
            << " with required configuration " << reqConfig << endl;
#endif //VPC_DEBUG
          
          this->tasksToProcess.push(currTask);
          this->runningTasks[currTask->getPID()] = currTask;
          // remove task that can be passed from waiting queue
          this->readyTasks.pop_front();
          
          // load new configuration
          this->nextConfiguration = reqConfig;

        }else{
          // current task not processable, so leave it and stop processing any further tasks
          break;
        }
      }
    }
  
   /*
  void FCFSController::addTasksToSchedule(std::deque<ProcessControlBlock* >& newTasks){
    this->waitInterval = NULL;

#ifdef VPC_DEBUG
        std::cerr << YELLOW("FCFSController "<< this->getName() <<"> addTasksToSchedule called! ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

    // first of all add tasks to local storage structure
    while(newTasks.size() > 0){
      this->readyTasks.push_back(newTasks.front());
      newTasks.pop_front();  
    }
    
    ProcessControlBlock* currTask;
     
    // now check which tasks to pass forward
    while(this->readyTasks.size()){
      currTask = this->readyTasks.front();

#ifdef VPC_DEBUG
      std::cerr << YELLOW("FCFSController "<< this->getName() <<"> processing task ") << currTask->getName() << endl;
#endif //VPC_DEBUG

      // get mapping for waiting task
      std::map<std::string, std::string >::iterator iter;
      iter = this->mapping_map_configs.find(currTask->getName());
      // check if mapping exists
      if(iter != this->mapping_map_configs.end()){
          
        Configuration* reqConf = this->getManagedComponent()->getConfiguration(iter->second.c_str());

        if(reqConf == NULL){

          // managed component does not support mapped configuration
          std::cerr << YELLOW("FCFSController "<< this->getName() << "> Configuration not supported by component") << std::endl;
// FIX ME -->            // until now pop task
          this->readyTasks.pop_front();
          continue;

        }

        // check if current activ configuration fits required one and no new configuration comes next
        if(reqConf == this->getManagedComponent()->getActivConfiguration() && this->nextConfiguration == NULL){

#ifdef VPC_DEBUG
          std::cerr << YELLOW("FCFSController "<< this->getName() <<"> current loaded configuration fits required one! ") << sc_simulation_time() << endl;
          std::cerr << YELLOW("FCFSController "<< this->getName() <<"> activ Configuration is activ? " << this->getManagedComponent()->getActivConfiguration()->isActiv()) << std::endl;
#endif //VPC_DEBUG

          this->tasksToProcess.push(currTask);
          this->runningTasks[currTask->getPID()] = currTask;

          // remove task that can be passed from waiting queue
          this->readyTasks.pop_front();
 
        }else  // check current configuration if it can be replaced
        if(this->runningTasks.size() == 0 || reqConf == this->nextConfiguration){

          this->tasksToProcess.push(currTask);
          this->runningTasks[currTask->getPID()] = currTask;
          // remove task that can be passed from waiting queue
          this->readyTasks.pop_front();
          
          // load new configuration
          //this->nextConfigurations.push(reqConf);
          this->nextConfiguration = reqConf;

        }else{
          // current task not processable, so leave it and stop processing any further tasks
          break;
        }
      }
    }
  }*/
   
  /*
   * \brief Implementation of FCFSController::getNextConfiguration
   */  
  unsigned int FCFSController::getNextConfiguration(){
    
    unsigned int next = this->nextConfiguration;
    this->nextConfiguration = 0;
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
  ProcessControlBlock* FCFSController::getNextTask(){
     
     ProcessControlBlock* task;
     task = this->tasksToProcess.front();
     this->tasksToProcess.pop();
     return task;
   
   }
  
  /**
   * \brief Implementation of FCFSController::signalTaskEvent
   */
  void FCFSController::signalTaskEvent(ProcessControlBlock* pcb){
  
#ifdef VPC_DEBUG
    std::cerr << "FCFSController " << this->getController().getName() << "> got notified by task: " << pcb->getName() << "::" << pcb->getFuncName()
              << " with running tasks num= " << this->runningTasks.size() << std::endl;
#endif //VPC_DEBUG
    
    this->runningTasks.erase(pcb->getPID());
    
    // if task has been killed and controlled instance is not killed solve decision here
    if(pcb->getState() == activation_state(aborted) && !this->getManagedComponent()->hasBeenKilled()){
      // recompute
      this->getManagedComponent()->compute(pcb);
    }else{
      this->getManagedComponent()->notifyParentController(pcb);
    }

#ifdef VPC_DEBUG
    if(pcb->getState() == activation_state(aborted)){
      std::cerr << YELLOW("FCFSController> task: " << pcb->getName() << " got killed!")  << std::endl;
    }
#endif //VPC_DEBUG

    // if there are no running task and still ready its time to wakeUp ReconfigurableComponent
    if(this->runningTasks.size() == 0 && this->readyTasks.size() != 0){
      // ensure that next configuration is reset
      this->nextConfiguration = 0;
#ifdef VPC_DEBUG
      std::cerr << "FCFSController> waking up component thread!" << std::endl;
#endif //VPC_DEBUG

      this->getManagedComponent()->wakeUp();
    }
  }

} //namespace SystemC_VPC
