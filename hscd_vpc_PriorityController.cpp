#include <hscd_vpc_PriorityController.h>

namespace SystemC_VPC{
  
  /**
   * \brief Initializes instance of PriorityController
   */
  PriorityController::PriorityController(const char* name) :
    Controller(name),
    order_count(0){}
  
  PriorityController::~PriorityController(){}
  
  /**
   * \brief Implementation of PreempetivController::addProcessToSchedule
   */
  void PriorityController::addProcessToSchedule(
    std::deque<ProcessControlBlock* >& newTasks)
  {
    this->waitInterval = NULL;

#ifdef VPC_DEBUG
        std::cerr << VPC_YELLOW("PriorityController "<< this->getName()
                  <<"> addProcessToSchedule called! ") << sc_simulation_time()
                  << endl;
#endif //VPC_DEBUG

    // add all task to processing list
    ProcessControlBlock* pcb;
    bool newelems = newTasks.size() > 0;
    
    while(newTasks.size() > 0){
      pcb = newTasks.front();
      this->tasksToProcess.push(pcb);
      // determine configuration and add priority of task to configuration
      std::map<std::string, std::string>::iterator iter =
        this->mapping_map_configs.find(pcb->getName());
      if(iter == this->mapping_map_configs.end()){
        std::cerr << VPC_RED("PriorityController " << this->getName()
                  << "> No mapped configuration found for " << pcb->getName())
                  << std::endl; 
      }else{
        //get configuration from managed component
        Configuration* config =
          this->getManagedComponent()->getConfiguration(iter->second.c_str());
        
        std::list<PriorityListElement<Configuration* > >::iterator iter;
        
        iter = std::find(this->nextConfigurations.begin(),
                         this->nextConfigurations.end(),
                         config);
        // if configuration is not in scheduling list add it
        if(iter == this->nextConfigurations.end()){
          this->nextConfigurations.push_back(
            PriorityListElement<Configuration* >( config,
                                                  pcb->getPriority(),
                                                  order_count++ ));
        }else{
          iter->addPriority(pcb->getPriority());
        }
        
      }
      
      newTasks.pop_front();  
    }
    
    if(newelems){
      this->nextConfigurations.sort();
    }
  }
   
  /*
   * \brief Implementation of PriorityController::getNextConfiguration
   */  
  Configuration* PriorityController::getNextConfiguration(){
    
    if(this->nextConfigurations.size()){
      Configuration* next = this->nextConfigurations.front().getContained();
    
      if(next != this->getManagedComponent()->getActivConfiguration()){

#ifdef VPC_DEBUG
        std::cerr << VPC_YELLOW("PriorityController " << this->getName()
                  << "> next config to load: "
                  << next->getName()) << std::endl;
#endif //VPC_DEBUG

        return next;
      }
    }
    return NULL;
  }
  
  /**
   * \brief Implementation of PriorityController::hasProcessToDispatch()
   */
  bool PriorityController::hasProcessToDispatch(){
  
    return (this->tasksToProcess.size() > 0);
  
  }
  
  /**
   * \brief Implementation of PriorityController::getNextProcess()
   */
  ProcessControlBlock* PriorityController::getNextProcess(){
     
     ProcessControlBlock* task;
     task = this->tasksToProcess.front();
     this->tasksToProcess.pop();
     return task;
   
   }
  
  /**
   * \brief Implementation of PriorityController::signalProcessEvent
   */
  void PriorityController::signalProcessEvent(ProcessControlBlock* pcb){
  
#ifdef VPC_DEBUG
    std::cerr << VPC_YELLOW("PriorityController " << this->getName()
              << "> got notified by task: " << pcb->getName()) << std::endl;
#endif //VPC_DEBUG
    
    //get mapped configuration
    Configuration* config =
      this->getMappedConfiguration((pcb->getName()).c_str());
    std::list<PriorityListElement<Configuration* > >::iterator iter;
        
    iter = std::find(this->nextConfigurations.begin(),
                     this->nextConfigurations.end(),
                     config);
    // if configuration is in scheduling list process it
    if(iter != this->nextConfigurations.end()){
      iter->removePriority(pcb->getPriority());
      
#ifdef VPC_DEBUG
      std::cerr << VPC_YELLOW("PriorityController " << this->getName()
                << "> priority of mapped configuration after change is: "
                << iter->getPriority()) << std::endl;
#endif //VPC_DEBUG
    
      // if no more task on configuration
      if(iter->getPriority() == -1){
        this->nextConfigurations.erase(iter);
      }else{
        this->nextConfigurations.sort(); //PriorityComparator()); 
      } 
    }
    
    // if task has been killed and controlled instance is not killed solve
    // decision here
    if( pcb->getState() == activation_state(aborted)
        && !this->getManagedComponent()->hasBeenKilled()){
      // recompute
      this->getManagedComponent()->compute(pcb);
    }else{
      this->getManagedComponent()->notifyParentController(pcb);
    }
        
#ifdef VPC_DEBUG
    if(pcb->getState() == activation_state(aborted)){
      std::cerr << VPC_YELLOW("PriorityController> task: " << pcb->getName()
                << " got killed!")  << std::endl;
    }
#endif //VPC_DEBUG
      
    // if there are still ready configurations and current activ does not fit
    // selected its time to wakeUp ReconfigurableComponent
    if( this->nextConfigurations.size() > 0
        && ( this->getManagedComponent()->getActivConfiguration()
             != this->getNextConfiguration() )){

#ifdef VPC_DEBUG
      std::cerr << "PriorityController> waking up component thread!"
                << std::endl;
#endif //VPC_DEBUG

      this->getManagedComponent()->wakeUp();
    }
  }
  
} //namespace SystemC_VPC
