#include "hscd_vpc_RREAllocator.h"

namespace SystemC_VPC {

  /**
   * SECTION Implementatio of RREConfElement
   */

  /**
   * \brief Implementation of RRConfElement::
   */
  RREConfElement::RREConfElement(unsigned int confID) : confID(confID) {}

  /**
   * \brief Implementation of RRConfElement::
   */
  RREConfElement::~RREConfElement() {}

  unsigned int RREConfElement::getID(){
    return this->confID;
  }

  /**
   * \brief Implementation of RRConfElement::
   */
  void RREConfElement::addTaskWQueue(ProcessControlBlock* pcb){
    //enqueue element
    this->waitingTasks.push_back(pcb);
  }

  /**
   * \brief Implementation of RRConfElement::
   */
  ProcessControlBlock* RREConfElement::processTask(){
    ProcessControlBlock* next = this->waitingTasks.front();
    this->waitingTasks.pop_front();
    this->runningTasks.push_back(next);
    
    assert(std::find(this->waitingTasks.begin(), this->waitingTasks.end(), next) == this->waitingTasks.end());
    
    return next;
  }

  /**
   * \brief Implementation of RRConfElement::
   */
  bool RREConfElement::hasWaitingTasks(){
    return (this->waitingTasks.size() > 0);
  }

  ProcessControlBlock* RREConfElement::getWaitingTask(){
    ProcessControlBlock* next = this->waitingTasks.front();
    this->waitingTasks.pop_front();
    
    assert(std::find(this->waitingTasks.begin(), this->waitingTasks.end(), next) == this->waitingTasks.end());
    
    return next;
  }

  /**
   * \brief Implementation of RRConfElement::
   */
  void RREConfElement::removeTask(ProcessControlBlock* pcb){
    
    // first check if pcb is contained in running tasks
    std::deque<ProcessControlBlock* >::iterator iter;
    iter = std::find(this->runningTasks.begin(), this->runningTasks.end(), pcb);
    if(iter != this->runningTasks.end()){
      ProcessControlBlock* pcb = *iter;
      this->runningTasks.erase(iter);
      assert(std::find(this->waitingTasks.begin(), this->waitingTasks.end(), pcb) == this->waitingTasks.end());
      assert(std::find(this->runningTasks.begin(), this->runningTasks.end(), pcb) == this->runningTasks.end());
      return;
    }
    // next check if in waiting tasks pcb is located, should be here if not in running tasks!!!
    iter = std::find(this->waitingTasks.begin(), this->waitingTasks.end(), pcb);
    if(iter != this->waitingTasks.end()){
      ProcessControlBlock* pcb = *iter;
      this->runningTasks.erase(iter);
      assert(std::find(this->waitingTasks.begin(), this->waitingTasks.end(), pcb) == this->waitingTasks.end());
      assert(std::find(this->runningTasks.begin(), this->runningTasks.end(), pcb) == this->runningTasks.end());
    }
  
  }

  /**
   * \brief Implementation of RRConfElement::
   */
  bool RREConfElement::RREConfElement::operator< (const RREConfElement& elem){

    int totalTasks1 = this->waitingTasks.size() + this->runningTasks.size();
    int totalTasks2 = elem.waitingTasks.size() + elem.runningTasks.size();

    if(totalTasks1 > totalTasks2){
      return true;
    }else{
      return false;
    }

    
  }

  int RREConfElement::getAssignedTaskCount(){
    return (this->waitingTasks.size() + this->runningTasks.size());
  }

  sc_time RREConfElement::getExecutionSum(){

    sc_time sum = SC_ZERO_TIME;
    
    std::deque<ProcessControlBlock* >::iterator iter;
    for(iter = this->runningTasks.begin(); iter != this->runningTasks.end(); iter++){
      sum += ((*iter)->getDelay() - (*iter)->getRemainingDelay());
    }
    
    return sum;
  }
  
  /**
   * SECTION Implementation of RREAllocator
   */


  /**
   * \brief Implementation of RRAllocator::
   */
  RREAllocator::RREAllocator(AbstractController* ctrl, double alpha) 
    : Allocator(ctrl),
      taskCount(0),
      lastassign(SC_ZERO_TIME),
      activTime(SC_ZERO_TIME),
      selected(NULL) {}

  /**
   * \brief Implementation of RRAllocator::
   */
  RREAllocator::~RREAllocator(){

    std::map<int, RREConfElement* >::iterator iter;
    for(iter = this->elems.begin(); iter != this->elems.end(); iter++){
      delete (iter->second);
    }

  }

  /**
   * \brief Implementation of RRAllocator::
   */
  void RREAllocator::addTaskToSchedule(ProcessControlBlock* newTask, unsigned int config, ReconfigurableComponent* rc){

//    std::cerr << "RREAllocator " << rc->basename() << "> addTaskToSchedule(" << newTask->getName() << ":" << newTask->getFuncName() << "::" << newTask->getPID() << " with requested conf=" << config << " at " << sc_simulation_time() << std::endl;
    
    RREConfElement* elem = NULL;

    // get management element
    std::map<int, RREConfElement* >::iterator iter;
    iter = this->elems.find(config);

    if(iter != this->elems.end()){
      elem = iter->second;
    }else{
      elem = new RREConfElement(config);
      this->elems.insert(std::pair<int, RREConfElement* >(config, elem));
//      std::cerr << "RREAllocator " << rc->basename() << "> addTaskToScheduler new management element added " << elem->getID() << std::endl;
    }

    // add it to waiting queue of configuration
    elem->addTaskWQueue(newTask);
    (this->taskCount)++;

    assert(this->elems[config]->getAssignedTaskCount() == elem->getAssignedTaskCount());
  }

  /**
   * \brief Implementation of RRAllocator::
   */
  void RREAllocator::performSchedule(ReconfigurableComponent* rc){

    // if no configuration has been scheduled yet! first run!
    if(this->selected == NULL){
//      std::cerr << "RREAllocator " << rc->basename() << "> no configuration selected yet!" << std::endl;

      // find configuration to allocate
      RREConfElement* next = NULL;
      // init next with first element of map
      std::map<int, RREConfElement* >::iterator iter;
      iter = this->elems.begin();
      next = iter->second;
      for(iter++; iter != this->elems.end(); iter++){
        if(*(iter->second) < *next){
          next = iter->second;
        }
      }

      this->setUpInitialParams(next, rc);

    }else{ // already scheduled configuration exists!

//      std::cerr << "RREAllocator " << rc->basename() << "> currently scheduled configuration=" << this->selected->getID() 
//        << " at " << sc_time_stamp() << std::endl;

      // update time variables
      // configuration was some considerable time already activ
      if(sc_time_stamp() > this->lastassign){
//        std::cerr << "RREAllocator " << rc->basename() << "> activTime before update=" << this->activTime << std::endl;
        this->activTime += (sc_time_stamp() - this->lastassign);
//        std::cerr << "RREAllocator " << rc->basename() << "> activTime updated to=" << this->activTime << " lastassign was=" << this->lastassign << " current timestamp= " << sc_time_stamp() << std::endl;
        this->lastassign = sc_time_stamp();
      }

      Configuration* c = rc->getActivConfiguration();
      assert(c->getID() == this->selected->getID());

      // check if any alternatives exist if not keep configuration
      if(this->selected->getAssignedTaskCount() == this->taskCount){
        // not so leave
//        std::cerr << "RREAllocator " << rc->basename() << "> no alternativ configuration have assigned tasks!" << std::endl;
        sc_time totalTimeToRun = (this->alpha * 2 * (c->getLoadTime() + c->getStoreTime()));

        if(totalTimeToRun > this->activTime){
          this->waitInterval = new sc_time((totalTimeToRun - this->activTime));
        }else{
          this->waitInterval = NULL;
        }

        return;
      }

      // check if condition for switching not reached yet!
      double proportion = (this->activTime / (2*(c->getLoadTime() + c->getStoreTime())));
      if(proportion < this->alpha && this->selected->getAssignedTaskCount() > 0){
        // needed proportion not reached, but still tasks to process
//        std::cerr << "RREAllocator " << rc->basename() << "> proportion=" << proportion << " and assigned tasks=" << this->selected->getAssignedTaskCount() << std::endl;
        sc_time totalTimeToRun = (this->alpha * 2 * (c->getLoadTime() + c->getStoreTime()));

        if(totalTimeToRun > this->activTime){
          this->waitInterval = new sc_time((totalTimeToRun - this->activTime));
        }else{
          this->waitInterval = NULL;
        }

        return;
      }

      // we have to switch configuration!
      // determine next configuration ... current will be ignored 
      RREConfElement* next = NULL;
      // init next with first element of map
      std::map<int, RREConfElement* >::iterator iter;
      for(iter = this->elems.begin(); iter != this->elems.end(); iter++){
        if(iter->second->getID() != this->selected->getID()){
          next = iter->second;
          break; // found first element to view
        }
      }
      // now find best fitting element
      for(iter++; iter != this->elems.end(); iter++){
        if(*(iter->second) < *next 
            && iter->second->getID() != this->selected->getID()){ //< dont forget to leave out currently selected config
          next = iter->second;
        }
      }

      // PANIK
      assert(next->getID() != this->selected->getID());

      this->setUpInitialParams(next, rc);

//      std::cerr << GREEN("RREAllocator> next = " << this->selected->getID() << " at " << sc_simulation_time() ) << std::endl;
    }

  }

  void RREAllocator::setUpInitialParams(RREConfElement* next, ReconfigurableComponent* rc){

    //set up initial parameters
    this->activTime = SC_ZERO_TIME;
    // add time to store and load if configurations of rc and selected are differing
    sc_time add_time = SC_ZERO_TIME;
    if(rc->getActivConfiguration() != NULL && rc->getActivConfiguration()->getID() != next->getID()){

      if(!this->killConfiguration(next, rc)){
        add_time += rc->getActivConfiguration()->getStoreTime();
        //std::cerr << "RREAllocator " << rc->basename() << "> add_time after adding storetime =" << add_time << std::endl;
      }
      
      add_time += rc->getConfiguration(next->getID())->getLoadTime();
      //std::cerr << "RREAllocator " << rc->basename() << "> add_time after adding loadtime =" << add_time << std::endl;

    }else
      if(rc->getActivConfiguration() == NULL){

        add_time += rc->getConfiguration(next->getID())->getLoadTime();
        //std::cerr << "RREAllocator " << rc->basename() << "> add_time after adding loadtime (no activ configuration!)=" << add_time << std::endl;

      }

    this->lastassign = sc_time_stamp() + add_time;
    //std::cerr << "RREAllocator " << rc->basename() << "> lastassign set to=" << this->lastassign << std::endl;

    this->selected = next;

    Configuration* c = rc->getConfiguration(this->selected->getID());

    this->waitInterval = new sc_time( (this->alpha * 2 * (c->getLoadTime() + c->getStoreTime()) + add_time) );  

  }
  
  /**
   * \brief Implementation of RRAllocator::
   */
  unsigned int RREAllocator::getNextConfiguration(ReconfigurableComponent* rc){
//    std::cerr << GREEN("RREAllocator> nextConfiguration = " << this->selected->getID() << " at ") << sc_simulation_time() << std::endl;
    return this->selected->getID();
  }

  /**
   * \brief Implementation of RRAllocator::
   */
  bool RREAllocator::hasTaskToProcess(ReconfigurableComponent* rc){
    if(selected != NULL){
      return this->selected->hasWaitingTasks();
    }else{
      return false;
    }
  }

  /**
   * \brief Implementation of RRAllocator::
   */
  ProcessControlBlock* RREAllocator::getNextTask(ReconfigurableComponent* rc){
    return this->selected->processTask();
  }

  /**
   * \brief Implementation of RRAllocator::
   */
  bool RREAllocator::setProperty(char* key, char* value){
    bool used = Allocator::setProperty(key, value);

    if(0 != std::strncmp(key, ALLOCATORPREFIX, strlen(ALLOCATORPREFIX))){
      std::cerr << ALLOCATORPREFIX << " != " << key << std::endl;
      return used;
    }else{
      key += strlen(ALLOCATORPREFIX);
    }
    
    std::cerr << "setProperty> " << key << " = " << value << std::endl;
    
    // we should do sth here for alpha;
    if(0==std::strncmp(key,"alpha",strlen("alpha"))){
      sscanf(value,"%lf",&alpha);
      used = true;
    }
    return used;
  }

  /**
   * \brief Implementation of RRAllocator::
   */
  void RREAllocator::signalPreemption(bool kill, ReconfigurableComponent* rc){
    // only on kill we have to abort all waiting tasks!
    if(kill){

      std::map<int, RREConfElement* >::iterator iter;
      RREConfElement* curr = NULL; 
      for(iter = this->elems.begin(); iter != this->elems.end(); iter++){
        curr = iter->second;
        //signal all waiting task of configuration as aborted
        while(curr->hasWaitingTasks()){
          ProcessControlBlock* task = curr->getWaitingTask();
          task->setState(activation_state(aborted));
          rc->notifyParentController(task);
        }

      }
      
      this->selected = NULL;  
    }else{
      this->activTime = this->activTime - (sc_time_stamp() - this->lastassign);
    }
  }

  /**
   * \brief Implementation of RRAllocator::
   */
  void RREAllocator::signalResume(ReconfigurableComponent* rc){
    this->lastassign = sc_time_stamp();
  }

  /**
   * \brief Implementation of RoundRobinAllocator::signalTaskEvent
   */
  void RREAllocator::signalTaskEvent(ProcessControlBlock* pcb, std::string compID){

//    std::cerr << "RREAllocator> got notified by task " << pcb->getName() << ":" << pcb->getPID() << " from " << compID << std::endl;
    
    Decision d = this->getController().getDecision(pcb->getPID(), this->getManagedComponent());
    std::map<int, RREConfElement* >::iterator iter;
    iter = this->elems.find(d.conf);
    if(iter != this->elems.end()){
//      std::cerr << "RREAllocator> found corresponding element!" << std::endl;
      iter->second->removeTask(pcb);
      (this->taskCount)--;
      
//      std::cerr << "RREAllocator> selected/scheduled conf=" << this->selected->getID() << " rc activ config=" << this->getManagedComponent()->getActivConfiguration()->getID() << std::endl;
//      std::cerr << "RREAllocator> current assigend tasks " << iter->second->getAssignedTaskCount() << " for conf=" << iter->second->getID() << std::endl;
      if(iter->second->getAssignedTaskCount() <= 0 && this->taskCount > 0){
//        std::cerr << "RREAllocator> waking up component!" << std::endl;
        this->getManagedComponent()->wakeUp();
      }

//      if(this->taskCount <= 0)
//        std::cerr << RED("RREAllocator " << compID << "> No more actually tasks! at " << sc_time_stamp() ) << std::endl;
      
    }

  }

  bool RREAllocator::killConfiguration(RREConfElement* elem, ReconfigurableComponent* rc){
    bool killConf = true;
    // only if there is already scheduled configuration!
    // else there should be no running task and so no need to store!
    if(elem != NULL){
      //determine if we can throw away calculation overhead
      //currently use fixed value beta
      double beta = 0.5;
      //sum up current execution times of each task on configuration
      sc_time execTime = elem->getExecutionSum(); //sc_time(elem->getExecutionSum(), SC_NS);
      Configuration* c = rc->getActivConfiguration();
      //sc_time reconfTime = (c->getLoadTime() + c->getStoreTime());

      killConf = ((execTime / c->getStoreTime()) < beta); //((execTime / reconfTime) < beta);
      this->setPreemptionStrategy(killConf);
    }
    return killConf;

  }

}

