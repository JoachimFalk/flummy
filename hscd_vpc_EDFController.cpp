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
  void EDFController::addTasksToSchedule(std::deque<p_struct* >& newTasks){
    this->waitInterval = NULL;

#ifdef VPC_DEBUG
        std::cerr << YELLOW("EDFController "<< this->getName() <<"> addTasksToSchedule called! ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

    // add all task to processing list
    p_struct* pcb;
    bool newElems = newTasks.size() > 0;
    
    while(newTasks.size() > 0){
      pcb = newTasks.front();
      this->tasksToProcess.push(pcb);
      // determine configuration and add EDF of task to configuration
      std::map<std::string, std::string>::iterator iter = this->mapping_map_configs.find(pcb->name);
      if(iter == this->mapping_map_configs.end()){
        std::cerr << RED("EDFController " << this->getName() << "> No mapped configuration found for " << pcb->name) << std::endl; 
      }else{
        //get configuration from managed component
        Configuration* config = this->getManagedComponent()->getConfiguration(iter->second.c_str());
        config->addDeadline(pcb->deadline);
        
        std::list<EDFListElement>::iterator iter;
        
        iter = std::find(this->nextConfigurations.begin(), this->nextConfigurations.end(), config);
        // if configuration is not in scheduling list add it
        if(iter == this->nextConfigurations.end()){
          this->nextConfigurations.push_back(EDFListElement(config, order_count++));
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
      Configuration* next = this->nextConfigurations.front().getConfiguration();
    
      if(next != this->getManagedComponent()->getActivConfiguration()){

#ifdef VPC_DEBUG
        std::cerr << YELLOW("EDFController " << this->getName() << "> next config to load: "
              << next->getName() << " with deadline= " << next->getDeadline()) << std::endl;
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
  p_struct* EDFController::getNextTask(){
     
     p_struct* task;
     task = this->tasksToProcess.front();
     this->tasksToProcess.pop();
     return task;
   
   }
  
  /**
   * \brief Implementation of EDFController::signalTaskEvent
   */
  void EDFController::signalTaskEvent(p_struct* pcb){
  
#ifdef VPC_DEBUG
    std::cerr << YELLOW("EDFController " << this->getName() << "> got notified by task: " << pcb->name) << std::endl;
#endif //VPC_DEBUG
    
    //get mapped configuration
    Configuration* config = this->getMappedConfiguration(pcb->name.c_str());
    config->removeDeadline(pcb->deadline);

#ifdef VPC_DEBUG
    std::cerr << YELLOW("EDFController " << this->getName() << "> deadline of mapped configuration after change is: "
          << config->getDeadline()) << std::endl;
#endif //VPC_DEBUG

    // if there are still tasks running on configuration equal to (EDF != -1)
    if(config->getDeadline() != -1){
      this->nextConfigurations.sort();
    }else{
      // remove configuration from EDF list
      std::list<EDFListElement>::iterator iter;
      iter = std::find(this->nextConfigurations.begin(), this->nextConfigurations.end(), config);
      if(iter != this->nextConfigurations.end()){
        this->nextConfigurations.erase(iter);
      }
    }
     
    // if task has been killed and controlled instance is not killed solve decision here
    if(pcb->state == activation_state(aborted) && !this->getManagedComponent()->hasBeenKilled()){
      // recompute
      this->getManagedComponent()->compute(pcb);
    }else{
      this->getManagedComponent()->notifyParentController(pcb);
    }
        
#ifdef VPC_DEBUG
    if(pcb->state == activation_state(aborted)){
      std::cerr << YELLOW("EDFController> task: " << pcb->name << " got killed!")  << std::endl;
    }
#endif //VPC_DEBUG
      
    // if there are no running task and still ready its time to wakeUp ReconfigurableComponent
    if(this->getManagedComponent()->getActivConfiguration()->getDeadline() == -1
        && this->nextConfigurations.size() > 0){

#ifdef VPC_DEBUG
      std::cerr << "EDFController> waking up component thread!" << std::endl;
#endif //VPC_DEBUG

      this->getManagedComponent()->wakeUp();
    }
  }
  
} //namespace SystemC_VPC
