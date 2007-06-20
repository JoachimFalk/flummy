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
   * \brief Implementation of PreempetivController::addProcessToSchedule
   */
  void FCFSController::addProcessToSchedule(
    std::deque<ProcessControlBlock* >& newTasks )
  {
    this->waitInterval = NULL;

#ifdef VPC_DEBUG
        std::cerr << VPC_YELLOW("FCFSController "<< this->getName()
                  << "> addProcessToSchedule called! ") << sc_simulation_time()
                  << std::endl;
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
      std::cerr << VPC_YELLOW("FCFSController "<< this->getName() 
                <<"> processing task ") << currTask->getName() << endl;
#endif //VPC_DEBUG

      // get mapping for waiting task
      std::map<std::string, std::string >::iterator iter;
      iter = this->mapping_map_configs.find(currTask->getName());
      // check if mapping exists
      if(iter != this->mapping_map_configs.end()){
          
        Configuration* reqConf =
          this->getManagedComponent()->getConfiguration(iter->second.c_str());

        if(reqConf == NULL){

          // managed component does not support mapped configuration
          std::cerr << VPC_YELLOW("FCFSController "<< this->getName()
                    << "> Configuration not supported by component")
                    << std::endl;
// FIX ME -->            // until now pop task
          this->readyTasks.pop_front();
          continue;

        }

        // check if current activ configuration fits required one and no new configuration comes next
        if( reqConf == this->getManagedComponent()->getActivConfiguration()
            && this->nextConfiguration == NULL ){

#ifdef VPC_DEBUG
          std::cerr << VPC_YELLOW("FCFSController "<< this->getName()
                    << "> current loaded configuration fits required one! ")
                    << sc_simulation_time() << endl;
          std::cerr << VPC_YELLOW("FCFSController "<< this->getName()
                    << "> activ Configuration is activ? "
                    << this->getManagedComponent()->getActivConfiguration()->
                                  isActiv())
                    << std::endl;
#endif //VPC_DEBUG

          this->tasksToProcess.push(currTask);
          this->runningTasks[currTask->getPID()] = currTask;

          // remove task that can be passed from waiting queue
          this->readyTasks.pop_front();
 
        }else  // check current configuration if it can be replaced
        if( this->runningTasks.size() == 0
            || reqConf == this->nextConfiguration ){

          this->tasksToProcess.push(currTask);
          this->runningTasks[currTask->getPID()] = currTask;
          // remove task that can be passed from waiting queue
          this->readyTasks.pop_front();
          
          // load new configuration
          //this->nextConfigurations.push(reqConf);
          this->nextConfiguration = reqConf;

        }else{
          // current task not processable, so leave it and stop processing any
          // further tasks
          break;
        }
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
   * \brief Implementation of FCFSController::hasProcessToDispatch()
   */
  bool FCFSController::hasProcessToDispatch(){
  
     return (this->tasksToProcess.size() > 0);
  
  }
  
  /**
   * \brief Implementation of FCFSController::getNextProcess()
   */
  ProcessControlBlock* FCFSController::getNextProcess(){
     
     ProcessControlBlock* task;
     task = this->tasksToProcess.front();
     this->tasksToProcess.pop();
     return task;
   
   }
  
  /**
   * \brief Implementation of FCFSController::signalProcessEvent
   */
  void FCFSController::signalProcessEvent(ProcessControlBlock* pcb){
  
#ifdef VPC_DEBUG
    std::cerr << "FCFSController " << this->getName()
              << "> got notified by task: " << pcb->getName() << "::"
              << pcb->getFuncName()
              << " with running tasks num= " << this->runningTasks.size()
              << std::endl;
#endif //VPC_DEBUG
    
    this->runningTasks.erase(pcb->getPID());
    
    // if task has been killed and controlled instance is not killed solve
    // decision here
    if( pcb->getState() == activation_state(aborted)
        && !this->getManagedComponent()->hasBeenKilled() ){
      // recompute
      this->getManagedComponent()->compute(pcb);
    }else{
      this->getManagedComponent()->notifyParentController(pcb);
    }

#ifdef VPC_DEBUG
    if(pcb->getState() == activation_state(aborted)){
      std::cerr << VPC_YELLOW("FCFSController> task: " << pcb->getName()
                << " got killed!")  << std::endl;
    }
#endif //VPC_DEBUG

    // if there are no running task and still ready its time to wakeUp
    // ReconfigurableComponent
    if(this->runningTasks.size() == 0 && this->readyTasks.size() != 0){

#ifdef VPC_DEBUG
      std::cerr << "FCFSController> waking up component thread!" << std::endl;
#endif //VPC_DEBUG

      this->getManagedComponent()->wakeUp();
    }
  }

  /**
   * \brief Implementation of FCFSController::signalDeallocation
   */
  void FCFSController::signalDeallocation(bool kill){
    // only interested in Deallocation with KILL
    if(kill){

      ProcessControlBlock* currTask;

      //for all waiting task signal their abortion
      while(this->readyTasks.size()){
        currTask = this->readyTasks.front();
        this->readyTasks.pop_front();
        currTask->setState(activation_state(aborted));
        this->getManagedComponent()->notifyParentController(currTask);
      }

    }
  }
  
} //namespace SystemC_VPC
