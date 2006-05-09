#include "hscd_vpc_RoundRobinConfScheduler.h"

namespace SystemC_VPC{
  
  /**
   * \brief Initializes instance of RoundRobinConfScheduler
   */
  RoundRobinConfScheduler::RoundRobinConfScheduler(AbstractController* controller) : ConfigurationScheduler(controller, NULL) {

    this->lastassign = -1;
    this->TIMESLICE = 1;
    this->scheduledConfiguration = NULL;
    
  }

  RoundRobinConfScheduler::~RoundRobinConfScheduler(){}

  /**
   * \brief Implementation of  RoundRobinConfScheduler::setProperty
   */
  bool RoundRobinConfScheduler::setProperty(char* key, char* value){

    bool used = ConfigurationScheduler::setProperty(key, value);

    if(0==strncmp(key,"timeslice",strlen("timeslice"))){

#ifdef VPC_DEBUG
      std::cerr << BLUE("RoundRobinConfScheduler> set property for timeslice = ") << value << std::endl;
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
   * \brief Implementation of RoundRobinConfScheduler::addTaskToSchedule
   */
  void RoundRobinConfScheduler::addTaskToSchedule(ProcessControlBlock* newTask, unsigned int config){

#ifdef VPC_DEBUG
        std::cerr << YELLOW("RoundRobinConfScheduler "<< this->getController().getName() <<"> addTasksToSchedule called! ") << sc_simulation_time() << endl;
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
   * \brief Implementation of RoundRobinConfScheduler::performSchedule
   */
  void RoundRobinConfScheduler::performSchedule(){
    
    delete this->waitInterval; 
    this->waitInterval = NULL;
    this->remainingSlice = this->remainingSlice - (sc_simulation_time() - this->lastassign);
    this->lastassign = sc_simulation_time();
  
    // configuration has to be switched if timeslice elapsed or no configuration scheduled yet
    if(this->remainingSlice <= 0 || this->scheduledConfiguration == NULL){

#ifdef VPC_DEBUG
        std::cerr << YELLOW("RoundRobinConfScheduler "<< this->getController().getName() <<"> timeslice elapsed at: ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

      this->switchConfig = true;
      
    }

    // as long as there are configs to schedule -> initiate awake of component some time later
    if(this->rr_configfifo.size() > 0){ //scheduledConfiguration != NULL){
        
#ifdef VPC_DEBUG
      std::cerr << YELLOW("RoundRobinConfScheduler "<< this->getController().getName() <<"> timeslice lasts: "
            << this->TIMESLICE-(sc_simulation_time()-this->lastassign) << " at: ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG
  
      this->waitInterval = new sc_time(this->TIMESLICE-(sc_simulation_time()-this->lastassign), true);
    }
  }
     
  /*
   * \brief Implementation of RoundRobinConfScheduler::getNextConfiguration
   */  
  unsigned int RoundRobinConfScheduler::getNextConfiguration(){

    unsigned int nextConfiguration = 0;

#ifdef VPC_DEBUG
    std::cerr << YELLOW("RoundRobinConfScheduler " << this->getController().getName() <<"> getNextConfiguration: switchConfig= " << this->switchConfig
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
      this->calculateAssignTime(nextConfiguration);
      this->remainingSlice = this->TIMESLICE;
    }
    
    // reset switchConfig
    this->switchConfig = false;        
    return nextConfiguration;
  }
   
  /**
   * \brief Implementation of RoundRobinConfScheduler::hasTaskToProcess()
   */
  bool RoundRobinConfScheduler::hasTaskToProcess(){
  
     return (this->tasksToProcess.size() > 0);
  
  }
  
  /**
   * \brief Implementation of RoundRobinConfScheduler::getNextTask()
   */
  ProcessControlBlock* RoundRobinConfScheduler::getNextTask(){
     
     ProcessControlBlock* task;
     task = this->tasksToProcess.front();
     this->tasksToProcess.pop();
     return task;
   
   }
  
  /**
   * \brief Implementation of RoundRobinConfScheduler::signalTaskEvent
   */
  void RoundRobinConfScheduler::signalTaskEvent(ProcessControlBlock* pcb){
  
#ifdef VPC_DEBUG
    std::cerr << "RoundRobinConfScheduler " << this->getController().getName() << "> got notified by task: " << pcb->getName() << std::endl;
#endif //VPC_DEBUG
    
    // remove running task out of registry
    this->updateUsedConfigurations(pcb);
    
    // if there are no running task on configuration wakeUp ReconfigurableComponent
    if(this->scheduledConfiguration == NULL && this->rr_configfifo.size() > 0){

#ifdef VPC_DEBUG
      std::cerr << "RoundRobinConfScheduler> waking up component thread!" << std::endl;
#endif //VPC_DEBUG

      this->getManagedComponent()->wakeUp();
    }
  }

  /**
   * \brief Implementation of RoundRobinConfScheduler::signalTaskEvent
   */  
  void RoundRobinConfScheduler::updateUsedConfigurations(ProcessControlBlock* pcb){
    
    Decision d = this->getController().getDecision(pcb->getPID());
    
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
      std::cerr << YELLOW("RoundRobinConfScheduler> configuration to be updated not in managed list!");
    }
    
  }
  
  /**
   * \brief Implementation of RoundRobinConfScheduler::signalTaskEvent
   */
  void RoundRobinConfScheduler::calculateAssignTime(unsigned int config){
    
    
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
    std::cerr << YELLOW("RoundRobinConfScheduler> time of last assignment set to: "<< this->lastassign) << std::endl;
#endif //VPC_DEBUG
  }
  
  /**
   * \brief Implementation of RoundRobinConfScheduler::signalTaskEvent
   */
  void RoundRobinConfScheduler::signalPreemption(){
    this->remainingSlice = this->remainingSlice - (sc_simulation_time() - this->lastassign);
  }

  /**
   * \brief Implementation of RoundRobinConfScheduler::signalTaskEvent
   */  
  void RoundRobinConfScheduler::signalResume(){
    this->lastassign = sc_simulation_time();
  }

} //namespace SystemC_VPC
