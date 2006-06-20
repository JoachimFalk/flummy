#include "hscd_vpc_RoundRobinAllocator.h"

namespace SystemC_VPC{
  
  /**
   * \brief Initializes instance of RoundRobinAllocator
   */
  RoundRobinAllocator::RoundRobinAllocator(AbstractController* controller) : Allocator(controller) {

    this->lastassign = -1;
    this->TIMESLICE = 1;
    this->scheduledConfiguration = NULL;
    
  }

  RoundRobinAllocator::~RoundRobinAllocator(){}

  /**
   * \brief Implementation of  RoundRobinAllocator::setProperty
   */
  bool RoundRobinAllocator::setProperty(char* key, char* value){

    bool used = Allocator::setProperty(key, value);

     // check if property relevant for allocator
    if(0 != std::strncmp(key, ALLOCATORPREFIX, strlen(ALLOCATORPREFIX))){
      return used;
    }else{
      key += strlen(ALLOCATORPREFIX);
    }
    
    if(0==strncmp(key,"timeslice",strlen("timeslice"))){

#ifdef VPC_DEBUG
      std::cerr << VPC_BLUE("RoundRobinAllocator> set property for timeslice = ") << value << std::endl;
#endif //VPC_DEBUG

      char *domain;
      domain=strstr(value,"ns");
      if(domain!=NULL){
        domain[0]='\0';
        sscanf(value,"%lf",&(this->TIMESLICE));
        used = true;
      }
    }

    return used;
  }
  
  /**
   * \brief Implementation of RoundRobinAllocator::addTaskToSchedule
   */
  void RoundRobinAllocator::addTaskToSchedule(ProcessControlBlock* newTask, unsigned int config, ReconfigurableComponent* rc){

#ifdef VPC_DEBUG
        std::cerr << VPC_YELLOW("RoundRobinAllocator "<< this->getController().getName() <<"> addTasksToSchedule called! ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG
    
    // add to internal structures
    this->tasksToProcess.push(newTask);
         
    // determine if new configuration has to be added to scheduling list
    std::deque<RRElement>::iterator  iter;
    iter = std::find(this->rr_configfifo.begin(), this->rr_configfifo.end(), RRElement(config));
    // if configuration not added yet do so
    if(iter == this->rr_configfifo.end()){
      this->rr_configfifo.push_back(RRElement(config, 1));
    }else{ // else increase count of running task on configuration
      (*iter)++;
    }
  }

  /**
   * \brief Implementation of RoundRobinAllocator::performSchedule
   */
  void RoundRobinAllocator::performSchedule(ReconfigurableComponent* rc){
    
    delete this->waitInterval; 
    this->waitInterval = NULL;
    assert(sc_simulation_time() >= this->lastassign);
    this->remainingSlice = this->remainingSlice - (sc_simulation_time() - this->lastassign);
    this->lastassign = sc_simulation_time();
  
    // configuration has to be switched if timeslice elapsed or no configuration scheduled yet
    if(this->remainingSlice <= 0 || this->scheduledConfiguration == NULL){

#ifdef VPC_DEBUG
        std::cerr << VPC_YELLOW("RoundRobinAllocator "<< this->getController().getName() <<"> timeslice elapsed at: ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

      this->switchConfig = true;
      
    }

    // as long as there are configs to schedule -> initiate awake of component some time later
    if(this->rr_configfifo.size() > 0){ //scheduledConfiguration != NULL){
        
#ifdef VPC_DEBUG
      std::cerr << VPC_YELLOW("RoundRobinAllocator "<< this->getController().getName() <<"> timeslice lasts: "
            << this->TIMESLICE-(sc_simulation_time()-this->lastassign) << " at: ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG
  
      this->waitInterval = new sc_time(this->TIMESLICE-(sc_simulation_time()-this->lastassign), true);
    }
  }
     
  /*
   * \brief Implementation of RoundRobinAllocator::getNextConfiguration
   */  
  unsigned int RoundRobinAllocator::getNextConfiguration(ReconfigurableComponent* rc){

    unsigned int nextConfiguration = 0;

#ifdef VPC_DEBUG
    std::cerr << VPC_YELLOW("RoundRobinAllocator " << this->getController().getName() <<"> getNextConfiguration: switchConfig= " << this->switchConfig
          << " fifo size= " << this->rr_configfifo.size() << "!") << std::endl;
#endif //VPC_DEBUG

    if(this->switchConfig && this->rr_configfifo.size() > 0){
      // put actual scheduled configuration to the end
      this->rr_configfifo.push_back(this->rr_configfifo.front());
      this->rr_configfifo.pop_front();
      // get configuration
      this->scheduledConfiguration = &(this->rr_configfifo.front());
      nextConfiguration = this->scheduledConfiguration->getConfiguration();
      
      // setup time of last assign
      this->calculateAssignTime(nextConfiguration, rc);
      this->remainingSlice = this->TIMESLICE;
    }
    
    // reset switchConfig
    this->switchConfig = false;        
    return nextConfiguration;
  }
   
  /**
   * \brief Implementation of RoundRobinAllocator::hasTaskToProcess()
   */
  bool RoundRobinAllocator::hasTaskToProcess(ReconfigurableComponent* rc){
  
     return (this->tasksToProcess.size() > 0);
  
  }
  
  /**
   * \brief Implementation of RoundRobinAllocator::getNextTask()
   */
  ProcessControlBlock* RoundRobinAllocator::getNextTask(ReconfigurableComponent* rc){
     
     ProcessControlBlock* task;
     task = this->tasksToProcess.front();
     this->tasksToProcess.pop();
     return task;
   
   }
  
  /**
   * \brief Implementation of RoundRobinAllocator::signalTaskEvent
   */
  void RoundRobinAllocator::signalTaskEvent(ProcessControlBlock* pcb, std::string compID){
  
#ifdef VPC_DEBUG
    std::cerr << "RoundRobinAllocator " << this->getController().getName() << "> got notified by task: " << pcb->getName() << std::endl;
#endif //VPC_DEBUG
    
    // remove running task out of registry
    this->updateUsedConfigurations(pcb, this->getManagedComponent());
    
    // if there are no running task on configuration wakeUp ReconfigurableComponent
    if(this->scheduledConfiguration == NULL && this->rr_configfifo.size() > 0){

#ifdef VPC_DEBUG
      std::cerr << "RoundRobinAllocator> waking up component thread!" << std::endl;
#endif //VPC_DEBUG

      this->getManagedComponent()->wakeUp();
    }
  }

  /**
   * \brief Implementation of RoundRobinAllocator::signalTaskEvent
   */  
  void RoundRobinAllocator::updateUsedConfigurations(ProcessControlBlock* pcb, ReconfigurableComponent* rc){
    
    Decision d = this->getController().getDecision(pcb->getPID(), rc);
    
    // update management structure
    std::deque<RRElement>::iterator iter;
    iter = std::find(this->rr_configfifo.begin(), this->rr_configfifo.end(), RRElement(d.conf));
    if(iter != this->rr_configfifo.end()){
      // if this task is last one running on configuration remove conf
      if(*iter == 1){
        if(this->scheduledConfiguration != NULL && iter->getConfiguration() == this->scheduledConfiguration->getConfiguration()){
          this->scheduledConfiguration = NULL;
        }
        this->rr_configfifo.erase(iter);
      }else{
        // else just decrease count of running task on configuration
        (*iter)--;
      }
    }else{
      std::cerr << VPC_YELLOW("RoundRobinAllocator> configuration to be updated not in managed list!");
    }
    
  }
  
  /**
   * \brief Implementation of RoundRobinAllocator::signalTaskEvent
   */
  void RoundRobinAllocator::calculateAssignTime(unsigned int config, ReconfigurableComponent* rc){
    
    
    Configuration* nextConfiguration =  this->getManagedComponent()->getConfiguration(config);
    this->lastassign = sc_simulation_time();
    
    if(this->getManagedComponent()->getActivConfiguration() != NULL 
        && nextConfiguration != this->getManagedComponent()->getActivConfiguration()){
      
      sc_time time;
      time = this->getManagedComponent()->getActivConfiguration()->timeToPreempt();
      this->lastassign += time.to_default_time_units();
      if(!this->preemptByKill()){
        this->lastassign += this->getManagedComponent()->getActivConfiguration()->getStoreTime().to_default_time_units();
      }
            
      time = nextConfiguration->timeToResume();
      this->lastassign += time.to_default_time_units();
        
      this->lastassign += nextConfiguration->getLoadTime().to_default_time_units();
    }
#ifdef VPC_DEBUG
    std::cerr << VPC_YELLOW("RoundRobinAllocator> time of last assignment set to: "<< this->lastassign) << std::endl;
#endif //VPC_DEBUG
  }
  
  /**
   * \brief Implementation of RoundRobinAllocator::signalTaskEvent
   */
  void RoundRobinAllocator::signalPreemption(bool kill, ReconfigurableComponent* rc){
    if(sc_simulation_time() > this->lastassign){
      this->remainingSlice = this->remainingSlice - (sc_simulation_time() - this->lastassign);
    }
  }

  /**
   * \brief Implementation of RoundRobinAllocator::signalTaskEvent
   */  
  void RoundRobinAllocator::signalResume(ReconfigurableComponent* rc){
    this->lastassign = sc_simulation_time();
  }

} //namespace SystemC_VPC
