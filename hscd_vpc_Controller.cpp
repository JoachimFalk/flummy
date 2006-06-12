#include "hscd_vpc_Controller.h"

namespace SystemC_VPC{
  
  Controller::Controller(const char* name) : waitInterval(NULL){
    
    strcpy(this->controllerName, name);
    
    char rest[VPC_MAX_STRING_LENGTH];
    int sublength;
    char *secondindex;
    char *firstindex=strchr(name,':');    //':' finden -> ':' trennt key-value Paare 
    while(firstindex!=NULL){
      secondindex=strchr(firstindex+1,':');        //':' �berspringen und n�chste ':' finden
      if(secondindex!=NULL)
        sublength=secondindex-firstindex;          //L�nge bestimmen
      else
        sublength=strlen(firstindex);              
      strncpy(rest,firstindex+1,sublength-1);      //key-value extrahieren
      rest[sublength-1]='\0';
      firstindex=secondindex;                     

      char *key, *value;               // key und value trennen und Property setzen
      value=strstr(rest,"-");
      if(value!=NULL){
        value[0]='\0';
        value++;
        key=rest;
        setProperty(key,value);
      }
    }
   
  }

  Controller::~Controller(){
 
    delete this->binder;
    delete this->scheduler;
    
  }
  
  /**
   * \brief Implementation of Controller::getName()
   */
  char* Controller::getName(){
    
    return this->controllerName;
    
  }
  
  AbstractBinder* Controller::getBinder(){
    return this->binder;
  }
  
  void Controller::setBinder(AbstractBinder* binder){
    if(binder != NULL){
      this->binder = binder;
    }
  }
    
  AbstractConfigurationMapper* Controller::getConfigurationMapper(){
    return this->mapper;
  }
  
  void Controller::setConfigurationMapper(AbstractConfigurationMapper* mapper){
    if(mapper != NULL){
      this->mapper = mapper;
    }
  }
    
  AbstractConfigurationScheduler* Controller::getConfigurationScheduler(){
    return this->scheduler;
  }
  
  void Controller::setConfigurationScheduler(AbstractConfigurationScheduler* scheduler){
    if(scheduler != NULL){
      this->scheduler = scheduler;
    }
  }
    
  /**
   * \brief Implementation of Controller::setManagedComponent
   */
  void Controller::setManagedComponent(ReconfigurableComponent* managedComponent){
    
    assert(managedComponent != NULL);
    this->managedComponent = managedComponent;
  
  }
  
  /**
   * \brief Implementation of Controller::getManagedComponent
   */
  ReconfigurableComponent* Controller::getManagedComponent(){
    
    return this->managedComponent;
    
  }
  
  /**
   * \brief Implementation of Controller::registerComponent
   */
  void Controller::registerComponent(AbstractComponent* comp){
    // do nothing right now
  }
      
  /**
   * \brief Implementation of Controller::registerMapping
   */
  /*
  void Controller::registerMapping(const char* taskName, const char* compName, MappingInformation* mInfo, AbstractComponent* rc){
    
#ifdef VPC_DEBUG
    std::cerr << "Controller> registerMapping(" << taskName << ", " << compName << ")" << std::endl;
#endif //VPC_DEBUG
    this->binder->registerBinding(taskName, compName);

  }*/
  
  /**
   * \brief Implementation of  Controller::setProperty
   */
  void Controller::setProperty(char* key, char* value){

    // just pass agruments on right now
    if(!this->binder->setProperty(key, value)
        && !this->mapper->setProperty(key, value)
        && !this->scheduler->setProperty(key, value)){

      std::cerr << VPC_YELLOW("Controller "<< this->getName() << "> Warning: Unkown property tag <" << key << "=" << value << "> , will be ignored! ") << std::endl;

    }
  }
  
  /**
   * \brief Implementation of Controller::addTasksToSchedule
   */
  void Controller::addTasksToSchedule(std::deque<ProcessControlBlock* >& newTasks, ReconfigurableComponent* rc){
    
    // process all new tasks
    while(newTasks.size() > 0){
      ProcessControlBlock* pcb = newTasks.front();
      // remember decisions for later use
      Decision d;
      d.pcb = pcb;

      // determine current binding of task
      d.comp = this->binder->resolveBinding(*pcb, rc);
      // determine corresponding configuration of bound component
      d.conf = this->mapper->getConfigForComp(d.comp);
      // store decision
      this->decisions[pcb->getPID()] = d;
      // register task and configuration for scheduling
      this->scheduler->addTaskToSchedule(pcb, d.conf, rc);
      newTasks.pop_front();  

    }
  }

  void Controller::performSchedule(ReconfigurableComponent* rc){ 
    this->scheduler->performSchedule(rc); 
     
  }

  bool Controller::hasTaskToProcess(ReconfigurableComponent* rc){
    return this->scheduler->hasTaskToProcess(rc);
  }
    
  ProcessControlBlock* Controller::getNextTask(ReconfigurableComponent* rc){
    return this->scheduler->getNextTask(rc);
  }
    
  unsigned int Controller::getNextConfiguration(ReconfigurableComponent* rc){
    return this->scheduler->getNextConfiguration(rc);
  }
  
  /**
   * \brief Implementation of Controller::getWaitInterval
   */
  sc_time* Controller::getWaitInterval(ReconfigurableComponent* rc){
    
    return this->scheduler->getWaitInterval(rc);
    
  }
     
  /**
   * \brief Implementation of Controller::getMappedComponent
   */
  AbstractComponent* Controller::getMappedComponent(ProcessControlBlock* task, ReconfigurableComponent* rc){
    
    std::map<int, Decision>::iterator iter;
    iter = this->decisions.find(task->getPID());
    if(iter != this->decisions.end()){
      Decision d = iter->second;
      return this->managedComponent->getConfiguration(d.conf)->getComponent(d.comp);
    }
    return NULL;
   
  }
  
  /**
    * \brief Dummy implementation of Controller::signalPreemption
    */  
  void Controller::signalPreemption(bool kill, ReconfigurableComponent* rc){
    this->scheduler->signalPreemption(kill, rc);
  }
  
  /**
    * \brief Dummy implementation of Controller::signalResume
    */
  void Controller::signalResume(ReconfigurableComponent* rc){
    this->scheduler->signalResume(rc);
  }
  
  /**
   * \brief Getter to determine which preemption mode is used
   */
  bool Controller::preemptByKill(){
    return this->scheduler->preemptByKill();
  }
  
  /**
   * \brief 
   */
  Decision Controller::getDecision(int pid, ReconfigurableComponent* rc) {
    return this->decisions[pid];
  }

  void Controller::signalTaskEvent(ProcessControlBlock* pcb, std::string compID){
    
    this->binder->signalTaskEvent(pcb, compID);
    this->scheduler->signalTaskEvent(pcb, compID);
  
     
    // if task has been killed and controlled instance is not killed solve decision here
    if(pcb->getState() == activation_state(aborted) && !this->managedComponent->hasBeenKilled()){
      // recompute
      this->managedComponent->compute(pcb);
    }else{
      this->managedComponent->notifyParentController(pcb);
    }
        
#ifdef VPC_DEBUG
    if(pcb->getState() == activation_state(aborted)){
      std::cerr << VPC_YELLOW("Controller " << this->getName() << "> task: " << pcb->getName() << " got killed!")  << std::endl;
    }
#endif //VPC_DEBUG
 
  }
 
}
