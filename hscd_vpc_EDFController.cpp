#include <hscd_vpc_EDFController.h>

namespace SystemC_VPC{
  
  /**
   * \brief Initializes instance of EDFController
   */
  EDFController::EDFController(const char* name) : Controller(name), order_count(0){}

  /**
   * \brief Deletes instance of EDFController
   */
  EDFController::~EDFController(){}
    
  /**
   * \brief Implementation of PreempetivController::addTasksToSchedule
   */
  void EDFController::addTasksToSchedule(std::deque<ProcessControlBlock* >& newTasks){
    this->waitInterval = NULL;

#ifdef VPC_DEBUG
        std::cerr << YELLOW("EDFController "<< this->getName() <<"> addTasksToSchedule called! ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

    // add all task to processing list
    ProcessControlBlock* pcb;
    bool newElems = newTasks.size() > 0;
    
    while(newTasks.size() > 0){
      pcb = newTasks.front();
      this->tasksToProcess.push(pcb);
      // determine configuration and add EDF of task to configuration
      std::map<std::string, std::string>::iterator iter = this->mapping_map_configs.find(pcb->getName());
      if(iter == this->mapping_map_configs.end()){
        std::cerr << RED("EDFController " << this->getName() << "> No mapped configuration found for " << pcb->getName()) << std::endl; 
      }else{
        //get configuration from managed component
        Configuration* config = this->getManagedComponent()->getConfiguration(iter->second.c_str());
        
        std::list<EDFListElement<Configuration* > >::iterator iter;
        
        iter = std::find(this->nextConfigurations.begin(), this->nextConfigurations.end(), config);
        // if configuration is not in scheduling list add it
        if(iter == this->nextConfigurations.end()){
          this->nextConfigurations.push_back(EDFListElement<Configuration* >(config, pcb->getDeadline(), order_count++));
        }else{
          iter->addDeadline(pcb->getDeadline());
        }
        
      }
      
      newTasks.pop_front();  
    }
    
    if(newElems) {
      //finally sort EDF list
      this->nextConfigurations.sort();
    }
  }
   
  /*
   * \brief Implementation of EDFController::getNextConfiguration
   */  
  Configuration* EDFController::getNextConfiguration(){
    
    if(!this->nextConfigurations.empty()){
      Configuration* next = this->nextConfigurations.front().getContained();
    
      if(next != this->getManagedComponent()->getActivConfiguration()){

#ifdef VPC_DEBUG
        std::cerr << YELLOW("EDFController " << this->getName() << "> next config to load: "
              << next->getName()) << std::endl;
#endif //VPC_DEBUG

        return next;
      }
    }
    return NULL;
  }
  
  /**
   * \brief Implementation of EDFController::hasTaskToProcess()
   */
  bool EDFController::hasTaskToProcess(){
   
     return (this->tasksToProcess.size() > 0);
  
  }
  
  /**
   * \brief Implementation of EDFController::getNextTask()
   */
  ProcessControlBlock* EDFController::getNextTask(){
     
     ProcessControlBlock* task;
     task = this->tasksToProcess.front();
     this->tasksToProcess.pop();
     return task;
   
   }
  
  /**
   * \brief Implementation of EDFController::signalTaskEvent
   */
  void EDFController::signalTaskEvent(ProcessControlBlock* pcb){
  
#ifdef VPC_DEBUG
    std::cerr << YELLOW("EDFController " << this->getName() << "> got notified by task: " << pcb->getName()) << std::endl;
#endif //VPC_DEBUG
    
    //get mapped configuration
    Configuration* config = this->getMappedConfiguration(pcb->getName().c_str());

    // if there are still tasks running on configuration equal to (EDF != -1)
    std::list<EDFListElement<Configuration* > >::iterator iter;
    iter = std::find(this->nextConfigurations.begin(), this->nextConfigurations.end(), config);

    if(iter != this->nextConfigurations.end()){
 
#ifdef VPC_DEBUG
    std::cerr << YELLOW("EDFController " << this->getName() << "> deadline of mapped configuration after change is: "
          << iter->getDeadline()) << std::endl;
#endif //VPC_DEBUG

     iter->removeDeadline(pcb->getDeadline());
     
     if(iter->getDeadline() != -1){
        this->nextConfigurations.sort();
      }else{
        // remove configuration from EDF list
        this->nextConfigurations.erase(iter);
      }
    }
     
    // if task has been killed and controlled instance is not killed solve decision here
    if(pcb->getState() == activation_state(aborted) && !this->getManagedComponent()->hasBeenKilled()){
      // recompute
      this->getManagedComponent()->compute(pcb);
    }else{
      this->getManagedComponent()->notifyParentController(pcb);
    }
        
#ifdef VPC_DEBUG
    if(pcb->getState() == activation_state(aborted)){
      std::cerr << YELLOW("EDFController> task: " << pcb->getName() << " got killed!")  << std::endl;
    }
#endif //VPC_DEBUG
      
    // if there are still waiting configurations and actual scheduled isnt determined one its time to wakeUp ReconfigurableComponent
    if(this->nextConfigurations.size() > 0 &&
        this->getManagedComponent()->getActivConfiguration() != this->getNextConfiguration()){

#ifdef VPC_DEBUG
      std::cerr << "EDFController> waking up component thread!" << std::endl;
#endif //VPC_DEBUG

      this->getManagedComponent()->wakeUp();
    }
  }
  
} //namespace SystemC_VPC
