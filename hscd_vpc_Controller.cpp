#include "hscd_vpc_Controller.h"

namespace SystemC_VPC{
  
  Controller::Controller(const char* name) : waitInterval(NULL){

    char *firstindex=strchr(name,':');    //':' finden -> ':' trennt key-value Paare 
    int sublength;
    
    if(firstindex != NULL){ 
      sublength = firstindex-name;
      strncpy(this->controllerName, name, sublength);
    }else{
      strcpy(this->controllerName, name);
    }
    
    char rest[VPC_MAX_STRING_LENGTH];
    char *secondindex;
    while(firstindex!=NULL){
      secondindex=strchr(firstindex+1,':');        //':' überspringen und nächste ':' finden
      if(secondindex!=NULL)
        sublength=secondindex-firstindex;          //Länge bestimmen
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
    delete this->allocator;
    
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
    
  AbstractAllocator* Controller::getAllocator(){
    return this->allocator;
  }
  
  void Controller::setAllocator(AbstractAllocator* allocator){
    if(allocator != NULL){
      this->allocator = allocator;
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
        && !this->allocator->setProperty(key, value)){

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
      this->allocator->addTaskToSchedule(pcb, d.conf, rc);
      newTasks.pop_front();  

    }
  }

  void Controller::performSchedule(ReconfigurableComponent* rc){ 
    this->allocator->performSchedule(rc); 
     
  }

  bool Controller::hasTaskToProcess(ReconfigurableComponent* rc){
    return this->allocator->hasTaskToProcess(rc);
  }
    
  ProcessControlBlock* Controller::getNextTask(ReconfigurableComponent* rc){
    return this->allocator->getNextTask(rc);
  }
    
  unsigned int Controller::getNextConfiguration(ReconfigurableComponent* rc){
    return this->allocator->getNextConfiguration(rc);
  }
  
  /**
   * \brief Implementation of Controller::getWaitInterval
   */
  sc_time* Controller::getWaitInterval(ReconfigurableComponent* rc){
    
    return this->allocator->getWaitInterval(rc);
    
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
    * \brief Dummy implementation of Controller::signalDeallocation
    */  
  void Controller::signalDeallocation(bool kill, ReconfigurableComponent* rc){
    this->allocator->signalDeallocation(kill, rc);
  }
  
  /**
    * \brief Dummy implementation of Controller::signalAllocation
    */
  void Controller::signalAllocation(ReconfigurableComponent* rc){
    this->allocator->signalAllocation(rc);
  }
  
  /**
   * \brief Getter to determine which preemption mode is used
   */
  bool Controller::deallocateByKill(){
    return this->allocator->deallocateByKill();
  }
  
  /**
   * \brief 
   */
  Decision Controller::getDecision(int pid, ReconfigurableComponent* rc) {
    return this->decisions[pid];
  }

  void Controller::signalProcessEvent(ProcessControlBlock* pcb, std::string compID){
    
    this->binder->signalProcessEvent(pcb, compID);
     
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

  sc_time Controller::getSchedulingOverhead(){
    sc_time t = SC_ZERO_TIME;
    t += this->binder->getBindingOverhead();
    t += this->allocator->getSchedulingOverhead();
    return t;
  }
  
}
