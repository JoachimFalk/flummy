#include <hscd_vpc_RoundRobinController.h>

namespace SystemC_VPC{
  
  /**
   * \brief Initializes instance of RoundRobinController
   */
  RoundRobinController::RoundRobinController(const char* name) :
    Controller(name) {

    this->lastassign = -1;
    this->TIMESLICE = 1;
    this->scheduledConfiguration = NULL;
    
  }

  RoundRobinController::~RoundRobinController(){}

  /**
   * \brief Implementation of  RoundRobinController::setProperty
   */
  void RoundRobinController::setProperty(char* key, char* value){

    Controller::setProperty(key, value);
    
    if(0==strncmp(key,"timeslice",strlen("timeslice"))){
      
#ifdef VPC_DEBUG
      std::cerr << VPC_BLUE("RoundRobinController> set property for timeslice"
                            " = ") << value << std::endl;
#endif //VPC_DEBUG
      
          char *domain;
          domain=strstr(value,"ns");
          if(domain!=NULL){
        domain[0]='\0';
        sscanf(value,"%lf",&(this->TIMESLICE));
          }
    }
  }
  
  /**
   * \brief Implementation of PreempetivController::addProcessToSchedule
   */
  void RoundRobinController::addProcessToSchedule(
   std::deque<ProcessControlBlock* >& newTasks)
  {
    this->waitInterval = NULL;

#ifdef VPC_DEBUG
        std::cerr << VPC_YELLOW("RoundRobinController "<< this->getName()
                  <<"> addProcessToSchedule called! ") << sc_simulation_time()
                  << endl;
#endif //VPC_DEBUG
    
    ProcessControlBlock* currTask = NULL;
    
    // add all tasks to running ones and processing list
    while(newTasks.size() > 0){
      currTask = newTasks.front();
      // add to internal structures
      this->tasksToProcess.push(currTask);
      
      newTasks.pop_front();
      
      // determine if new configuration has to be added to scheduling list
      std::string confid = this->mapping_map_configs[currTask->getName()];
      Configuration* reqConf =
        this->getManagedComponent()->getConfiguration(confid.c_str());
    
      std::deque<RRElement>::iterator  iter;
      iter = std::find(this->rr_configfifo.begin(),
                       this->rr_configfifo.end(),
                       RRElement(reqConf));

      // if configuration not added yet do so
      if(iter == this->rr_configfifo.end()){
        this->rr_configfifo.push_back(RRElement(reqConf, 1));
      }else{ // else increase count of running task on configuration
        (*iter)++;
      }
    
    }
    
    this->remainingSlice = this->remainingSlice -
      (sc_simulation_time() - this->lastassign);
    this->lastassign = sc_simulation_time();
  
    // configuration has to be switched if timeslice elapsed or no
    // configuration scheduled yet
    if(this->remainingSlice <= 0 || this->scheduledConfiguration == NULL){

#ifdef VPC_DEBUG
        std::cerr << VPC_YELLOW("RoundRobinController "<< this->getName()
                  << "> timeslice elapsed at: ") << sc_simulation_time()
                  << endl;
#endif //VPC_DEBUG

      this->switchConfig = true;
      
    }

    // as long as there are configs to schedule
    // -> initiate awake of component some time later
    if(this->scheduledConfiguration != NULL){
        
#ifdef VPC_DEBUG
      std::cerr << VPC_YELLOW("RoundRobinController "<< this->getName()
                << "> timeslice lasts: "
                << this->TIMESLICE-(sc_simulation_time()-this->lastassign)
                << " at: ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG
  
      this->waitInterval =
        new sc_time( this->TIMESLICE-(sc_simulation_time()-this->lastassign),
                     true);
    }
  }
   
  /*
   * \brief Implementation of RoundRobinController::getNextConfiguration
   */  
  Configuration* RoundRobinController::getNextConfiguration(){

    Configuration* nextConfiguration = NULL;

#ifdef VPC_DEBUG
    std::cerr << VPC_YELLOW("RoundRobinController " << this->getName()
              << "> getNextConfiguration: switchConfig= " << this->switchConfig
              << " fifo size= " << this->rr_configfifo.size() << "!")
              << std::endl;
#endif //VPC_DEBUG

    if(this->switchConfig && this->rr_configfifo.size() > 0){
      // put actual scheduled configuration to the end
      this->rr_configfifo.push_back(this->rr_configfifo.front());
      this->rr_configfifo.pop_front();
      // get configuration
      this->scheduledConfiguration = &(this->rr_configfifo.front());
      nextConfiguration = this->scheduledConfiguration->getConfiguration();
      
      // setup time of last assign
      this->calculateAssignTime(nextConfiguration);
      this->remainingSlice = this->TIMESLICE;
    }
    
    // reset switchConfig
    this->switchConfig = false;        
    return nextConfiguration;
  }
   
  /**
   * \brief Implementation of RoundRobinController::hasProcessToDispatch()
   */
  bool RoundRobinController::hasProcessToDispatch(){
  
     return (this->tasksToProcess.size() > 0);
  
  }
  
  /**
   * \brief Implementation of RoundRobinController::getNextProcess()
   */
  ProcessControlBlock* RoundRobinController::getNextProcess(){
     
     ProcessControlBlock* task;
     task = this->tasksToProcess.front();
     this->tasksToProcess.pop();
     return task;
   
   }
  
  /**
   * \brief Implementation of RoundRobinController::signalProcessEvent
   */
  void RoundRobinController::signalProcessEvent(ProcessControlBlock* pcb){
  
#ifdef VPC_DEBUG
    std::cerr << "RoundRobinController " << this->getName()
              << "> got notified by task: " << pcb->getName() << std::endl;
#endif //VPC_DEBUG
    
    // remove running task out of registry
    this->updateUsedConfigurations(pcb);
    
    // if task has been killed and
    // controlled instance is not killed solve decision here
    if( pcb->getState() == activation_state(aborted)
        && !this->getManagedComponent()->hasBeenKilled() ){
      // recompute
      this->getManagedComponent()->compute(pcb);
    }else{
      this->getManagedComponent()->notifyParentController(pcb);
    }
        
#ifdef VPC_DEBUG
    if(pcb->getState() == activation_state(aborted)){
      std::cerr << VPC_YELLOW("RoundRobinController> task: " << pcb->getName()
                << " got killed!")  << std::endl;
    }
#endif //VPC_DEBUG
      
    // if there are no running task on configuration wakeUp
    // ReconfigurableComponent
    if(this->scheduledConfiguration == NULL && this->rr_configfifo.size() > 0){

#ifdef VPC_DEBUG
      std::cerr << "RoundRobinController> waking up component thread!"
                << std::endl;
#endif //VPC_DEBUG

      this->getManagedComponent()->wakeUp();
    }
  }

  /**
   * \brief Implementation of RoundRobinController::signalProcessEvent
   */  
  void RoundRobinController::updateUsedConfigurations(ProcessControlBlock* pcb)
  {
    
    std::deque<RRElement>::iterator iter;
    std::string confid = this->mapping_map_configs[pcb->getName()];
    Configuration* conf =
      this->getManagedComponent()->getConfiguration(confid.c_str());
    
    // update management structure
    iter = std::find(this->rr_configfifo.begin(),
                     this->rr_configfifo.end(),
                     RRElement(conf));
    if(iter != this->rr_configfifo.end()){
      // if this task is last one running on configuration remove conf
      if(*iter == 1){
        if( this->scheduledConfiguration != NULL
            && ( iter->getConfiguration() ==
                 this->scheduledConfiguration->getConfiguration()) ){
          this->scheduledConfiguration = NULL;
        }
        this->rr_configfifo.erase(iter);
      }else{
        // else just decrease count of running task on configuration
        (*iter)--;
      }
    }else{
      std::cerr << VPC_YELLOW("RoundRobinController> configuration to be"
                              " updated not in managed list!");
    }
  
  }
  
  /**
   * \brief Implementation of RoundRobinController::calculateAssignTime
   */
  void RoundRobinController::calculateAssignTime(
    Configuration* nextConfiguration)
  {
    
    this->lastassign = sc_simulation_time();
    
    if( nextConfiguration != 
        this->getManagedComponent()->getActivConfiguration() ){

      sc_time time;
      if(this->getManagedComponent()->getActivConfiguration() != NULL){
        time = this->getManagedComponent()->getActivConfiguration()->
          timeToDeallocate();
        this->lastassign += time.to_default_time_units();
        if(!this->deallocateByKill()){
          this->lastassign += this->getManagedComponent()->
            getActivConfiguration()->getStoreTime().to_default_time_units();
        }
      }
      
      time = nextConfiguration->timeToAllocate();
      this->lastassign += time.to_default_time_units();
        
      this->lastassign +=
        nextConfiguration->getLoadTime().to_default_time_units();
    }
    
#ifdef VPC_DEBUG
    std::cerr << VPC_YELLOW("RoundRobinController> time of last assignment"
                            " set to: "<< this->lastassign) << std::endl;
#endif //VPC_DEBUG
  }
  
  /**
   * \brief Implementation of RoundRobinController::signalDeallocation
   */
  void RoundRobinController::signalDeallocation(bool kill){
    this->remainingSlice =
      this->remainingSlice - (sc_simulation_time() - this->lastassign);
  }

  /**
   * \brief Implementation of RoundRobinController::signalAllocation
   */  
  void RoundRobinController::signalAllocation(){
    this->lastassign = sc_simulation_time();
  }

} //namespace SystemC_VPC
