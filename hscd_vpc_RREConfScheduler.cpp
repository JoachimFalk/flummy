#include "hscd_vpc_RREConfScheduler.h"

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
      this->runningTasks.erase(iter);
      assert(std::find(this->waitingTasks.begin(), this->waitingTasks.end(), *iter) == this->waitingTasks.end());
      assert(std::find(this->runningTasks.begin(), this->runningTasks.end(), *iter) == this->runningTasks.end());
      return;
    }
    // next check if in waiting tasks pcb is located, should be here if not in running tasks!!!
    iter = std::find(this->waitingTasks.begin(), this->waitingTasks.end(), pcb);
    if(iter != this->waitingTasks.end()){
      this->waitingTasks.erase(iter);
    }
    assert(std::find(this->waitingTasks.begin(), this->waitingTasks.end(), *iter) == this->waitingTasks.end());
    assert(std::find(this->runningTasks.begin(), this->runningTasks.end(), *iter) == this->runningTasks.end());
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

  /**
   * SECTION Implementation of RREConfScheduler
   */


  /**
   * \brief Implementation of RRConfScheduler::
   */
  RREConfScheduler::RREConfScheduler(AbstractController* ctrl, MIMapper* miMapper, double alpha) 
    : ConfigurationScheduler(ctrl, miMapper),
      taskCount(0),
      lastassign(SC_ZERO_TIME),
      activTime(SC_ZERO_TIME),
      selected(NULL) {}

  /**
   * \brief Implementation of RRConfScheduler::
   */
  RREConfScheduler::~RREConfScheduler(){

    std::map<int, RREConfElement* >::iterator iter;
    for(iter = this->elems.begin(); iter != this->elems.end(); iter++){
      delete (iter->second);
    }

  }

  /**
   * \brief Implementation of RRConfScheduler::
   */
  void RREConfScheduler::addTaskToSchedule(ProcessControlBlock* newTask, unsigned int config, ReconfigurableComponent* rc){

//    std::cerr << "RREConfScheduler " << rc->basename() << "> addTaskToSchedule(" << newTask->getName() << ":" << newTask->getPID() 
//      << " with requested conf=" << config << " from " << rc->basename() << std::endl;
    
    RREConfElement* elem = NULL;

    // get management element
    std::map<int, RREConfElement* >::iterator iter;
    iter = this->elems.find(config);

    if(iter != this->elems.end()){
      elem = iter->second;
    }else{
      elem = new RREConfElement(config);
      this->elems.insert(std::pair<int, RREConfElement* >(config, elem));
//      std::cerr << "RREConfScheduler " << rc->basename() << "> addTaskToScheduler new management element added " << elem->getID() << std::endl;
    }

    // add it to waiting queue of configuration
    elem->addTaskWQueue(newTask);
    (this->taskCount)++;

    assert(this->elems[config]->getAssignedTaskCount() == elem->getAssignedTaskCount());
  }

  /**
   * \brief Implementation of RRConfScheduler::
   */
  void RREConfScheduler::performSchedule(ReconfigurableComponent* rc){

    // if no configuration has been scheduled yet! first run!
    if(this->selected == NULL){
//      std::cerr << "RREConfScheduler " << rc->basename() << "> no configuration selected yet!" << std::endl;

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
/*      
      //set up initial parameters
      this->selected = next;
      this->activTime = SC_ZERO_TIME;
      // add time to store and load if configurations of rc and selected are differing
      sc_time add_time = SC_ZERO_TIME;
      if(rc->getActivConfiguration() != NULL && rc->getActivConfiguration()->getID() != this->selected->getID()){

        add_time += rc->getActivConfiguration()->getStoreTime();
//        std::cerr << "RREConfScheduler " << rc->basename() << "> add_time after adding storetime =" << add_time << std::endl;
        add_time += rc->getConfiguration(this->selected->getID())->getLoadTime();
//        std::cerr << "RREConfScheduler " << rc->basename() << "> add_time after adding loadtime =" << add_time << std::endl;

      }else
        if(rc->getActivConfiguration() == NULL){

          add_time += rc->getConfiguration(this->selected->getID())->getLoadTime();
//          std::cerr << "RREConfScheduler " << rc->basename() << "> add_time after adding loadtime (no activ configuration!)=" << add_time << std::endl;

        }

      this->lastassign = sc_time_stamp() + add_time;
//      std::cerr << "RREConfScheduler " << rc->basename() << "> lastassign set to=" << this->lastassign << std::endl;

      Configuration* c = rc->getConfiguration(this->selected->getID());

      this->waitInterval = new sc_time( (this->alpha * 2 * (c->getLoadTime() + c->getStoreTime()) + add_time) );  
*/
    }else{ // already scheduled configuration exists!

//      std::cerr << "RREConfScheduler " << rc->basename() << "> currently scheduled configuration=" << this->selected->getID() 
//        << " at " << sc_time_stamp() << std::endl;

      // update time variables
      // configuration was some considerable time already activ
      if(sc_time_stamp() > this->lastassign){
//        std::cerr << "RREConfScheduler " << rc->basename() << "> activTime before update=" << this->activTime << std::endl;
        this->activTime += (sc_time_stamp() - this->lastassign);
//        std::cerr << "RREConfScheduler " << rc->basename() << "> activTime updated to=" << this->activTime << " lastassign was=" << this->lastassign << " current timestamp= " << sc_time_stamp() << std::endl;
        this->lastassign = sc_time_stamp();
      }

      Configuration* c = rc->getActivConfiguration();
      assert(c->getID() == this->selected->getID());

      // check if any alternatives exist if not keep configuration
      if(this->selected->getAssignedTaskCount() == this->taskCount){
        // not so leave
//        std::cerr << "RREConfScheduler " << rc->basename() << "> no alternativ configuration have assigned tasks!" << std::endl;
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
//        std::cerr << "RREConfScheduler " << rc->basename() << "> proportion=" << proportion << " and assigned tasks=" << this->selected->getAssignedTaskCount() << std::endl;
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
    }
/*
      //set up initial parameters
      this->selected = next;
      this->activTime = SC_ZERO_TIME;

      // add time to store and load if configurations of rc and selected are differing
      sc_time add_time = SC_ZERO_TIME;
      if(rc->getActivConfiguration() != NULL && rc->getActivConfiguration()->getID() != this->selected->getID()){

        add_time += rc->getActivConfiguration()->getStoreTime();
//        std::cerr << "RREConfScheduler " << rc->basename() << "> add_time after adding storetime =" << add_time << std::endl;
        add_time += rc->getConfiguration(this->selected->getID())->getLoadTime();
//        std::cerr << "RREConfScheduler " << rc->basename() << "> add_time after adding loadtime =" << add_time << std::endl;

      }else
        if(rc->getActivConfiguration() == NULL){

          add_time += rc->getConfiguration(this->selected->getID())->getLoadTime();
//          std::cerr << "RREConfScheduler " << rc->basename() << "> add_time after adding loadtime (no activ configuration!)=" << add_time << std::endl;

        }

      this->lastassign = sc_time_stamp() + add_time;
//      std::cerr << "RREConfScheduler " << rc->basename() << "> lastassign set to=" << this->lastassign << std::endl;

      c = rc->getConfiguration(this->selected->getID());

      this->waitInterval = new sc_time( (this->alpha * 2 * (c->getLoadTime() + c->getStoreTime()) + add_time) );  
    }
    */
  }

  void RREConfScheduler::setUpInitialParams(RREConfElement* next, ReconfigurableComponent* rc){

    //set up initial parameters
    this->selected = next;
    this->activTime = SC_ZERO_TIME;
    // add time to store and load if configurations of rc and selected are differing
    sc_time add_time = SC_ZERO_TIME;
    if(rc->getActivConfiguration() != NULL && rc->getActivConfiguration()->getID() != this->selected->getID()){

      add_time += rc->getActivConfiguration()->getStoreTime();
      //std::cerr << "RREConfScheduler " << rc->basename() << "> add_time after adding storetime =" << add_time << std::endl;
      add_time += rc->getConfiguration(this->selected->getID())->getLoadTime();
      //std::cerr << "RREConfScheduler " << rc->basename() << "> add_time after adding loadtime =" << add_time << std::endl;

    }else
      if(rc->getActivConfiguration() == NULL){

        add_time += rc->getConfiguration(this->selected->getID())->getLoadTime();
        //std::cerr << "RREConfScheduler " << rc->basename() << "> add_time after adding loadtime (no activ configuration!)=" << add_time << std::endl;

      }

    this->lastassign = sc_time_stamp() + add_time;
    //std::cerr << "RREConfScheduler " << rc->basename() << "> lastassign set to=" << this->lastassign << std::endl;

    Configuration* c = rc->getConfiguration(this->selected->getID());

    this->waitInterval = new sc_time( (this->alpha * 2 * (c->getLoadTime() + c->getStoreTime()) + add_time) );  

  }
  
  /**
   * \brief Implementation of RRConfScheduler::
   */
  unsigned int RREConfScheduler::getNextConfiguration(ReconfigurableComponent* rc){
    if(this->selected != NULL){
      return this->selected->getID();
    }else{
      return 0;
    }
  }

  /**
   * \brief Implementation of RRConfScheduler::
   */
  bool RREConfScheduler::hasTaskToProcess(ReconfigurableComponent* rc){
    if(selected != NULL){
      return this->selected->hasWaitingTasks();
    }else{
      return false;
    }
  }

  /**
   * \brief Implementation of RRConfScheduler::
   */
  ProcessControlBlock* RREConfScheduler::getNextTask(ReconfigurableComponent* rc){
    return this->selected->processTask();
  }

  /**
   * \brief Implementation of RRConfScheduler::
   */
  bool RREConfScheduler::setProperty(char* key, char* value){
    bool used = ConfigurationScheduler::setProperty(key, value);
    // we should do sth here for alpha;
    if(0==strncmp(key,"alpha",strlen("alpha"))){
      sscanf(value,"%lf",&alpha);
      used = true;
    }
    return used;
  }

  /**
   * \brief Implementation of RRConfScheduler::
   */
  void RREConfScheduler::signalPreemption(bool kill, ReconfigurableComponent* rc){
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
   * \brief Implementation of RRConfScheduler::
   */
  void RREConfScheduler::signalResume(ReconfigurableComponent* rc){
    this->lastassign = sc_time_stamp();
  }

  /**
   * \brief Implementation of RoundRobinConfScheduler::signalTaskEvent
   */
  void RREConfScheduler::signalTaskEvent(ProcessControlBlock* pcb, std::string compID){

//    std::cerr << "RREConfScheduler> got notified by task " << pcb->getName() << ":" << pcb->getPID() << " from " << compID << std::endl;
    
    Decision d = this->getController().getDecision(pcb->getPID(), this->getManagedComponent());
    std::map<int, RREConfElement* >::iterator iter;
    iter = this->elems.find(d.conf);
    if(iter != this->elems.end()){
//      std::cerr << "RREConfScheduler> found corresponding element!" << std::endl;
      iter->second->removeTask(pcb);
      (this->taskCount)--;
      
//      std::cerr << "RREConfScheduler> selected/scheduled conf=" << this->selected->getID() << " rc activ config=" << this->getManagedComponent()->getActivConfiguration()->getID() << std::endl;
//      std::cerr << "RREConfScheduler> current assigend tasks " << iter->second->getAssignedTaskCount() << " for conf=" << iter->second->getID() << std::endl;
      if(iter->second->getAssignedTaskCount() <= 0 && this->taskCount > 0){
//        std::cerr << "RREConfScheduler> waking up component!" << std::endl;
        this->getManagedComponent()->wakeUp();
      }

      if(this->taskCount <= 0)
        std::cerr << RED("RREConfScheduler " << compID << "> No more actually tasks! at " << sc_time_stamp() ) << std::endl;
      
    }

  }

  bool RREConfScheduler::killConfiguration(){
    return this->preemptByKill();
  }

}

