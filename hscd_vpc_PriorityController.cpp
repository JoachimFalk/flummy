#include <hscd_vpc_PriorityController.h>

namespace SystemC_VPC{
  
  /**
   * \brief Initializes instance of PriorityController
   */
  PriorityController::PriorityController(const char* name) : order_count(0){

    strcpy(this->controllerName, name);
    
  }

  PriorityController::~PriorityController(){}
  
  /**
   * \brief Implementation of PreempetivController::addTasksToSchedule
   */
  void PriorityController::addTasksToSchedule(std::deque<p_struct* >& newTasks){
    this->waitInterval = NULL;

#ifdef VPC_DEBUG
        std::cerr << YELLOW("PriorityController "<< this->getName() <<"> addTasksToSchedule called! ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

    // add all task to processing list
    p_struct* pcb;
    bool newElems = newTasks.size() > 0;
    
    while(newTasks.size() > 0){
      pcb = newTasks.front();
      this->tasksToProcess.push(pcb);
      // determine configuration and add priority of task to configuration
      std::map<std::string, std::string>::iterator iter = this->mapping_map_configs.find(pcb->name);
      if(iter == this->mapping_map_configs.end()){
        std::cerr << RED("PriorityController " << this->getName() << "> No mapped configuration found for " << pcb->name) << std::endl; 
      }else{
        //get configuration from managed component
        Configuration* config = this->managedComponent->getConfiguration(iter->second.c_str());
        config->addPriority(pcb->priority);
        
        std::list<PriorityListElement>::iterator iter;
        
        iter = std::find(this->nextConfigurations.begin(), this->nextConfigurations.end(), config);
        // if configuration is not in scheduling list add it
        if(iter == this->nextConfigurations.end()){
          this->nextConfigurations.push_back(PriorityListElement(config, order_count++));
        }
        
      }
      
      newTasks.pop_front();  
    }
    
    if(newElems) {
      //finally sort priority list
      this->nextConfigurations.sort();
    }
  }
   
  /*
   * \brief Implementation of PriorityController::getNextConfiguration
   */  
    Configuration* PriorityController::getNextConfiguration(){
    
    if(!this->nextConfigurations.empty()){
      Configuration* next = this->nextConfigurations.front().getConfiguration();
    
      if(next != this->managedComponent->getActivConfiguration()){

#ifdef VPC_DEBUG
        std::cerr << YELLOW("PriorityController " << this->getName() << "> next config to load: "
              << next->getName() << " with priority= " << next->getPriority()) << std::endl;
#endif //VPC_DEBUG

        return next;
      }
    }
    return NULL;
  }
  
  /**
   * \brief Implementation of PriorityController::hasTaskToProcess()
   */
  bool PriorityController::hasTaskToProcess(){
  
     return (this->tasksToProcess.size() > 0);
  
  }
  
  /**
   * \brief Implementation of PriorityController::getNextTask()
   */
  p_struct* PriorityController::getNextTask(){
     
     p_struct* task;
     task = this->tasksToProcess.front();
     this->tasksToProcess.pop();
     return task;
   
   }
  
  /**
   * \brief Implementation of PriorityController::signalTaskEvent
   */
  void PriorityController::signalTaskEvent(p_struct* pcb){
  
#ifdef VPC_DEBUG
    std::cerr << YELLOW("PriorityController " << this->getName() << "> got notified by task: " << pcb->name) << std::endl;
#endif //VPC_DEBUG
    
    //get mapped configuration
    Configuration* config = this->getMappedConfiguration(pcb->name.c_str());
    config->removePriority(pcb->priority);

#ifdef VPC_DEBUG
    std::cerr << YELLOW("PriorityController " << this->getName() << "> priority of mapped configuration after change is: "
          << config->getPriority()) << std::endl;
#endif //VPC_DEBUG

    // if there are still tasks running on configuration equal to (priority != -1)
    if(config->getPriority() != -1){
      this->nextConfigurations.sort();
    }else{
      // remove configuration from priority list
      std::list<PriorityListElement>::iterator iter;
      iter = std::find(this->nextConfigurations.begin(), this->nextConfigurations.end(), config);
      if(iter != this->nextConfigurations.end()){
        this->nextConfigurations.erase(iter);
      }
    }
     
    // if task has been killed and controlled instance is not killed solve decision here
    if(pcb->state == activation_state(aborted) && !this->managedComponent->hasBeenKilled()){
      // recompute
      this->managedComponent->compute(pcb);
    }else{
      this->managedComponent->notifyParentController(pcb);
    }
        
#ifdef VPC_DEBUG
    if(pcb->state == activation_state(aborted)){
      std::cerr << YELLOW("PriorityController> task: " << pcb->name << " got killed!")  << std::endl;
    }
#endif //VPC_DEBUG
      
    // if there are no running task and still ready its time to wakeUp ReconfigurableComponent
    if(this->managedComponent->getActivConfiguration()->getPriority() == -1
        && this->nextConfigurations.size() > 0){

#ifdef VPC_DEBUG
      std::cerr << "PriorityController> waking up component thread!" << std::endl;
#endif //VPC_DEBUG

      this->managedComponent->wakeUp();
    }
  }
  
} //namespace SystemC_VPC
