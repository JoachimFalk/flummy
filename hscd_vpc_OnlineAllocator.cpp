#include <hscd_vpc_OnlineAllocator.h>
//#define VPC_DEBUG
namespace SystemC_VPC{
  
  /**
   * \brief Initializes instance of OnlineAllocator
   */
  OnlineAllocator::OnlineAllocator(AbstractController* controller) : Allocator(controller){
  
    this->nextConfiguration = 0;
    
  }

  /**
   * \brief Deletes instance of OnlineAllocator
   */
  OnlineAllocator::~OnlineAllocator(){
  
    assert(this->readyTasks.size() == 0);
    assert(this->runningTasks.size() == 0);
    assert(this->tasksToProcess.size() == 0);
    
  }
  
  /**
   * \brief Implementation of OnlineAllocator::addProcessToSchedule
   */
  void OnlineAllocator::addProcessToSchedule(ProcessControlBlock* newTask, unsigned int config, ReconfigurableComponent* rc){

#ifdef VPC_DEBUG
        std::cerr << VPC_YELLOW("OnlineAllocator "<< this->getController().getName() <<"> addProcessToSchedule called! ") 
          << "For task " << newTask->getName() << " with required configuration id " << config << " at " << sc_simulation_time() << endl;
//        std::cerr << VPC_YELLOW("OnlineAllocator "<< this->getController().getName() <<"> addTasksToSchedule called! ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG
    // first of all add task to local storage structure
    std::pair<ProcessControlBlock*, unsigned int> entry(newTask, config);
    this->readyTasks.push_back(entry);
  }

  /**
   * \brief Implementation of OnlineAllocator::performSchedule
   */
  void OnlineAllocator::performSchedule(ReconfigurableComponent* rc){

    ProcessControlBlock* currTask;
    unsigned int reqConfig = 0;
    // now check which tasks to pass forward
    while(this->readyTasks.size()){
      currTask = this->readyTasks.front().first;
      reqConfig = this->readyTasks.front().second;

#ifdef VPC_DEBUG
      std::cerr << VPC_YELLOW("OnlineAllocator "<< this->getController().getName() <<"> processing task ") << currTask->getName()
        << " with required configuration id " << reqConfig << endl;
      if(this->getManagedComponent()->getActivConfiguration() == NULL){
        std::cerr << VPC_YELLOW("OnlineAllocator "<< this->getController().getName() <<"> no activ configuration for managed component ") << endl;
      }else{
        std::cerr << VPC_YELLOW("OnlineAllocator "<< this->getController().getName() <<"> activ configuration for managed component is ") 
          << this->getManagedComponent()->getActivConfiguration()->getName() << " with id " 
          << this->getManagedComponent()->getActivConfiguration()->getID() << endl;
        std::cerr << VPC_YELLOW("OnlineAllocator "<< this->getController().getName() <<"> currently running num of tasks on conf ") << this->runningTasks.size() << endl;
      }
      std::cerr << VPC_YELLOW("OnlineAllocator "<< this->getController().getName() <<"> next configuration is set to ") << this->nextConfiguration << endl;
#endif //VPC_DEBUG

      // check if
      if((this->getManagedComponent()->getActivConfiguration() == NULL && this->nextConfiguration == 0) // no activ and no next conf selected
          || (this->runningTasks.size() == 0 && this->nextConfiguration == 0) // or no running tasks and no next conf selected
          || (this->nextConfiguration == 0 && reqConfig == this->getManagedComponent()->getActivConfiguration()->getID()) // or no selected conf but actual one fits!
          || reqConfig == this->nextConfiguration)
        { // or required conf fits already selected one

#ifdef VPC_DEBUG
        std::cerr << VPC_YELLOW("OnlineAllocator "<< this->getController().getName() <<"> can process task ") << currTask->getName() 
          << " with required configuration " << reqConfig << endl;
#endif //VPC_DEBUG

        this->tasksToProcess.push(currTask);
        this->runningTasks[currTask->getPID()] = currTask;
        // remove task that can be passed from waiting queue
        this->readyTasks.pop_front();

        // load new configuration
        this->nextConfiguration = reqConfig;
        
        //blocked because of other SetupTime
        //this->waitInterval = getBlockedTime();
        //this->nextConfiguration = 0;
      }else{
        // current task not processable, so leave it and stop processing any further tasks
        break;
      }
    }
  }
  
  /**
   * \brief Implementation of OnlineAllocator::getNextConfiguration
   */  
  unsigned int OnlineAllocator::getNextConfiguration(ReconfigurableComponent* rc){
    
    unsigned int next = this->nextConfiguration;
    this->nextConfiguration = 0;
    return next;
  
  }
   
  /**
   * \brief Implementation of OnlineAllocator::hasProcessToDispatch()
   */
  bool OnlineAllocator::hasProcessToDispatch(ReconfigurableComponent* rc){
  
     return (this->tasksToProcess.size() > 0);
  
  }
  
  /**
   * \brief Implementation of OnlineAllocator::getNextProcess()
   */
  ProcessControlBlock* OnlineAllocator::getNextProcess(ReconfigurableComponent* rc){
     
     ProcessControlBlock* task;
     task = this->tasksToProcess.front();
     this->tasksToProcess.pop();
     return task;
   
   }
  
  /**
   * \brief Implementation of OnlineAllocator::signalProcessEvent
   */
  void OnlineAllocator::signalProcessEvent(ProcessControlBlock* pcb, std::string compID){
  
#ifdef VPC_DEBUG
    std::cerr << "OnlineAllocator " << this->getController().getName() << "> got notified by task: " << pcb->getName() << "::" << pcb->getFuncName()
              << " with running tasks num= " << this->runningTasks.size() << std::endl;
#endif //VPC_DEBUG
    
    this->runningTasks.erase(pcb->getPID());
    
    // if there are no running task and still ready its time to wakeUp ReconfigurableComponent
    if(this->runningTasks.size() == 0 && this->readyTasks.size() != 0){
      // ensure that next configuration is reset
      this->nextConfiguration = 0;
#ifdef VPC_DEBUG
      std::cerr << "OnlineAllocator> waking up component thread!" << std::endl;
#endif //VPC_DEBUG

      this->getManagedComponent()->wakeUp();
    }
  }

  /**
   * \brief Implementation of OnlineAllocator::signalDeallocation
   */
  void OnlineAllocator::signalDeallocation(bool kill, ReconfigurableComponent* rc){
    // only interested in Preemption with KILL
    if(kill){
      
      std::pair<ProcessControlBlock*, unsigned int> p;
      ProcessControlBlock* currTask;

      //for all waiting task signal their abortion
      while(this->readyTasks.size()){
        p = this->readyTasks.front();
        currTask = p.first;
        this->readyTasks.pop_front();
        currTask->setState(activation_state(aborted));
        this->getManagedComponent()->notifyParentController(currTask);
      }

    }
  }
  /**
   * \brief Implementation of OnlineAllocator::getSetuptime()
   */
   sc_time OnlineAllocator::getSetuptime(ProcessControlBlock* task){
     ReconfigurableComponent* myComp = this->getManagedComponent();
     Configuration* myConf = myComp->getConfiguration(task->getPID());
     if(!myConf)std::cerr << "Conf ist NULL" << std::endl;
     return myConf->getLoadTime();
   }
   
  /**
   * \brief Implementation of OnlineAllocator::getSchedulingOverhead()
   */
  sc_time OnlineAllocator::getSchedulingOverhead(){

    return SC_ZERO_TIME;
    
  }
  
  /**
   * \brief Implementation of OnlineAllocator::setBlockedTime, called by Binder to block concurrent configuration
   */
  void OnlineAllocator::setBlockedTime(sc_time time){
    //this->waitInterval = &time;
  }
  
  /**
   * \brief Implementation of OnlineAllocator::cleanstring, helper of generate_sctime
   */
  void OnlineAllocator::cleanstring(std::string *output){
    std::string::iterator iter = output->begin();
        while(*iter == ' ' || *iter == '\t' ) {
          iter = output->erase(iter);
        }
        iter = output->end()-1;
        while(*iter == ' ' || *iter == '\t') {
          output->erase(iter);
          iter = output->end()-1;
        }
    return;
  }
  
  /**
   * \brief Implementation of OnlineAllocator::generate_sctime
   */
  sc_time OnlineAllocator::generate_sctime(std::string starttime){
    //trenne Zahl und einheit
    std::string numbers = "0123456789.";
    std::string::iterator iter = starttime.begin();
    while( numbers.find(*iter) != std::string::npos) iter++;
    std::string time = starttime.substr(0, iter - starttime.begin());
    std::string unit = starttime.substr(iter - starttime.begin(), starttime.end() - iter);
    
    //std::cerr << "time:"<<time<<"Unit:"<<unit<<std::endl;
      
    std::istringstream timex;
    double timeindouble;
    timex.str( time );
    timex >> timeindouble;
    
    cleanstring(&unit);
    //generiere sc_time(zahl,einheit)
    sc_time_unit scUnit = SC_NS;
    if(      0==unit.compare(0, 2, "fs") ) scUnit = SC_FS;
    else if( 0==unit.compare(0, 2, "ps") ) scUnit = SC_PS;
    else if( 0==unit.compare(0, 2, "ns") ) scUnit = SC_NS;
    else if( 0==unit.compare(0, 2, "us") ) scUnit = SC_US;
    else if( 0==unit.compare(0, 2, "ms") ) scUnit = SC_MS;
    else if( 0==unit.compare(0, 1, "s" ) ) scUnit = SC_SEC;
    
    return sc_time(timeindouble,scUnit);
  }
} //namespace SystemC_VPC
