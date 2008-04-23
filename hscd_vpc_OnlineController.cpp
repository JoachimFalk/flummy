#include "hscd_vpc_OnlineController.h"
#define VPC_DEBUG
namespace SystemC_VPC {
  
  /**
   * \brief Initializes instance of OnlineController
   */
  OnlineController::OnlineController(AbstractController* controller) : DynamicBinder(), Allocator(controller){
  
    this->nextConfiguration = 0;
    numberofcomp = 0;
    config_blocked_until = sc_time_stamp();
  }

  /**
   * \brief Deletes instance of OnlineController
   */
  OnlineController::~OnlineController(){
  
    assert(this->readyTasks.size() == 0);
    assert(this->runningTasks.size() == 0);
    assert(this->tasksToProcess.size() == 0);
  }
  
  /**
   * \brief Implemetation of OnlineController::performBinding
   */
  std::pair<std::string, MappingInformation* > OnlineController::performBinding(ProcessControlBlock& task, ReconfigurableComponent* comp)
  throw(UnknownBindingException){
    
    //Get access to Components to count them
    Binding* b = NULL;
    if(comp == NULL){
      b = task.getBindingGraph().getRoot();
    }else{
      b = task.getBindingGraph().getBinding(comp->basename());
    }
    ChildIterator* bIter = b->getChildIterator();
    
    if(numberofcomp == 0){
      ChildIterator* counter = bIter;
      while(counter->hasNext()){
        counter->getNext();
        rctime.push_back(generate_sctime("0ms"));
        numberofcomp++;
      }
      bIter = b->getChildIterator();
    }
    std::cerr << "LOADTIME: " << b->getID()<<std::endl;
#ifdef VPC_DEBUG  
    std::cerr << "**************************************"<< std::endl;
    std::cerr << "OnlineController> sc_time_stamp: " << sc_time_stamp() << endl;
    std::cerr << "OnlineController> numberofcomp "<< numberofcomp << std::endl;
#endif    

#ifdef VPC_DEBUG
    if (comp != NULL) std::cerr << "OnlineController> Component: "<< comp->basename() <<"> Task: " << task.getName() << endl;
    if (comp == NULL) std::cerr << "OnlineController> Component: NULL"<<"> Task: " << task.getName() << endl;
#endif

    //check queue for rc with minimum runtime left
    int chosen = 0;
    for(int i=0; i < numberofcomp; i++){
      if (rctime[i] < rctime[chosen])
        chosen = i;
    }
    //jump to chosen Recomponent
    for(int i=0; i <= chosen; i++){
      if(bIter->hasNext()){
        b = bIter->getNext();
      }else{    
        delete bIter;
        std::string msg = "OnlineController> No target specified for "+ task.getName() +"->?";
        throw UnknownBindingException(msg);
      }
    }
    delete bIter;
    //getMappingInformation
    MappingInformationIterator* iter = b->getMappingInformationIterator();
    if(iter->hasNext()){
      MappingInformation* mInfo = iter->getNext();
      delete iter;

      //Runtime + Mapping
#ifdef VPC_DEBUG
      std::cerr << "OnlineController> Runtime: " << mInfo->getDelay() << endl;   
      std::cerr << "OnlineController> Chose Mapping: "<< b->getID() << endl;
#endif        
      
      //getSetupTime
      //Director* myDir = dynamic_cast<Director*>(getDirector());
      Director* myDir = &Director::getInstance();
      //ReconfigurableComponent* myComp = myDir->getReComp();
      std::string aReComp =
        task.getBindingGraph().getRoot()->getChildIterator()->getNext()->getID();
      ReconfigurableComponent* myComp = myDir->getCompByName(aReComp);

      if(myComp == NULL){
        std::cerr << "OnlineController> MyComp ist NULL" << std::endl;
      }
      Binding* myBinding = task.getBindingGraph().getBinding(myComp->basename());
      ChildIterator* mybIter = myBinding->getChildIterator();
      if (mybIter->hasNext()) myBinding = mybIter->getNext();
      //unsigned int ConfID = myComp->getConfigurationPool().getConfigForComp(task.getBindingGraph().getRoot()->getID());
      //Configuration* myConfig = myComp->getConfigurationPool().getConfigByID(ConfID);
      
      AbstractController* myCtrl = myComp->getController();
      if(myCtrl == NULL){
        std::cerr << "OnlineController> MyCtrl ist NULL" << std::endl;
      }
      unsigned int ConfID = myCtrl->getConfigurationMapper()->getConfigForComp(myBinding->getID());
      
      Configuration* myConfig = myComp->getConfiguration(ConfID);
      
      // determine current binding of process
      //d.comp = this->binder->resolveBinding(*pcb, rc);
      // determine corresponding configuration of bound component
      //d.conf = this->mapper->getConfigForComp(d.comp);
      sc_time setuptime = myConfig->getLoadTime();
#ifdef VPC_DEBUG      
      std::cerr << "OnlineController> SetupTime: "<< setuptime << std::endl;
#endif
      
      //move all slots time-border for next possible configuration by this->setuptime
      sc_time chosentime = rctime[chosen];
      for(int i=0; i<numberofcomp; i++){
        if(rctime[i] < (chosentime+setuptime))
          rctime[i] = chosentime+setuptime;
          if(i == chosen)
            rctime[chosen] += mInfo->getDelay();
#ifdef VPC_DEBUG
        std::cerr << "OnlineController> time-border for Slot" << i+1 << " = " << rctime[i] << std::endl;
#endif
      }
    
      //wait till last configuration finished
      if (sc_time_stamp() < config_blocked_until){
        wait(config_blocked_until - sc_time_stamp());
        //myAll->setBlockedTime(config_blocked_until - sc_time_stamp());
      }
      //Statt wait hier, myAll->setBlockedTime: PROBLEM, nur eine ReComponente geladen, Director müsste alle schicken
      config_blocked_until = sc_time_stamp() + setuptime;     
#ifdef VPC_DEBUG
      std::cerr << "OnlineController> config_blocked_until: " << config_blocked_until << endl;
#endif            
    //return MappingInformation
    return std::pair<std::string, MappingInformation*>(b->getID(), mInfo);
      
    }else{
      // also free iterator
      delete iter;
      return std::pair<std::string, MappingInformation*>("", NULL);
    }
  }//end of OnlineController::performBinding()

  /**
   * \brief Helper for OnlineController::generate_sctime
   */
  void OnlineController::cleanstring(std::string *output){
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
   * \brief Implementation of OnlineController::generate_sctime
   */
  sc_time OnlineController::generate_sctime(std::string starttime){
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
  
  /**
   * \brief Implementation of OnlineController::addProcessToSchedule
   */
  void OnlineController::addProcessToSchedule(ProcessControlBlock* newTask, unsigned int config, ReconfigurableComponent* rc){

#ifdef VPC_DEBUG
        std::cerr << VPC_YELLOW("OnlineController> addProcessToSchedule called! ") 
          << "For task " << newTask->getName() << " with required configuration id " << config << " at " << sc_simulation_time() << endl;
//        std::cerr << VPC_YELLOW("OnlineController "<< this->getController().getName() <<"> addTasksToSchedule called! ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG
    // first of all add task to local storage structure
    std::pair<ProcessControlBlock*, unsigned int> entry(newTask, config);
    this->readyTasks.push_back(entry);
  }

  /**
   * \brief Implementation of OnlineController::performSchedule
   */
  void OnlineController::performSchedule(ReconfigurableComponent* rc){

    ProcessControlBlock* currTask;
    unsigned int reqConfig = 0;
    // now check which tasks to pass forward
    while(this->readyTasks.size()){
      currTask = this->readyTasks.front().first;
      reqConfig = this->readyTasks.front().second;

#ifdef VPC_DEBUG
      std::cerr << VPC_YELLOW("OnlineController> processing task ") << currTask->getName()
        << " with required configuration id " << reqConfig << endl;
      if(this->getManagedComponent()->getActivConfiguration() == NULL){
        std::cerr << VPC_YELLOW("OnlineController> no activ configuration for managed component ") << endl;
      }else{
        std::cerr << VPC_YELLOW("OnlineController> activ configuration for managed component is ") 
          << this->getManagedComponent()->getActivConfiguration()->getName() << " with id " 
          << this->getManagedComponent()->getActivConfiguration()->getID() << endl;
        std::cerr << VPC_YELLOW("OnlineController> currently running num of tasks on conf ") << this->runningTasks.size() << endl;
      }
      std::cerr << VPC_YELLOW("OnlineController> next configuration is set to ") << this->nextConfiguration << endl;
#endif //VPC_DEBUG

      // check if
      if((this->getManagedComponent()->getActivConfiguration() == NULL && this->nextConfiguration == 0) // no activ and no next conf selected
          || (this->runningTasks.size() == 0 && this->nextConfiguration == 0) // or no running tasks and no next conf selected
          || (this->nextConfiguration == 0 && reqConfig == this->getManagedComponent()->getActivConfiguration()->getID()) // or no selected conf but actual one fits!
          || reqConfig == this->nextConfiguration)
        { // or required conf fits already selected one

#ifdef VPC_DEBUG
        std::cerr << VPC_YELLOW("OnlineController> can process task ") << currTask->getName() 
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
   * \brief Implementation of OnlineController::getNextConfiguration
   */  
  unsigned int OnlineController::getNextConfiguration(ReconfigurableComponent* rc){
    
    unsigned int next = this->nextConfiguration;
    this->nextConfiguration = 0;
    return next;
  }
   
  /**
   * \brief Implementation of OnlineController::hasProcessToDispatch()
   */
  bool OnlineController::hasProcessToDispatch(ReconfigurableComponent* rc){
  
     return (this->tasksToProcess.size() > 0);
  }
  
  /**
   * \brief Implementation of OnlineController::getNextProcess()
   */
  ProcessControlBlock* OnlineController::getNextProcess(ReconfigurableComponent* rc){
     
     ProcessControlBlock* task;
     task = this->tasksToProcess.front();
     this->tasksToProcess.pop();
     return task;
   }
  
  /**
   * \brief Implementation of OnlineController::signalProcessEvent
   */
  void OnlineController::signalProcessEvent(ProcessControlBlock* pcb, std::string compID){
  
#ifdef VPC_DEBUG
    std::cerr << "OnlineController> got notified by task: " << pcb->getName() << "::" << pcb->getFuncName()
              << " with running tasks num= " << this->runningTasks.size() << std::endl;
#endif //VPC_DEBUG
    
    this->runningTasks.erase(pcb->getPID());
    
    // if there are no running task and still ready its time to wakeUp ReconfigurableComponent
    if(this->runningTasks.size() == 0 && this->readyTasks.size() != 0){
      // ensure that next configuration is reset
      this->nextConfiguration = 0;
#ifdef VPC_DEBUG
      std::cerr << "OnlineController> waking up component thread!" << std::endl;
#endif //VPC_DEBUG

      this->getManagedComponent()->wakeUp();
    }
  }

  /**
   * \brief Implementation of OnlineController::signalDeallocation
   */
  void OnlineController::signalDeallocation(bool kill, ReconfigurableComponent* rc){
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
   * \brief Implementation of OnlineController::getSchedulingOverhead()
   */
  sc_time OnlineController::getSchedulingOverhead(){

    sc_time myTime = generate_sctime("10ms"); //VPCBuilder::createSC_Time
    if(this->tasksToProcess.size() > 0){
      ReconfigurableComponent* myComp = this->getManagedComponent();
      Configuration* myConf = myComp->getConfiguration(this->nextConfiguration);
      myTime = myConf->getLoadTime();
    }
    else if(this->readyTasks.size()){
      ReconfigurableComponent* myComp = this->getManagedComponent();
      Configuration* myConf = myComp->getConfiguration(this->readyTasks.front().second);
      myTime = myConf->getLoadTime();
    }
    else{
#ifdef VPC_DEBUG    
      std::cerr << "OnlineController::getSchedulingOverhead> No task found" << std::endl;
#endif
    }
    
#ifdef VPC_DEBUG        
    std::cerr << "OnlineController::getSchedulingOverhead> loadtime: " << myTime << std::endl;
#endif
    //Wenn von Recomponent aufgerufen, return blockingtime durch andere Recomp die gerade Configern
    return myTime;
  }
  
  /**
   * \brief Implementation of OnlineController::setBlockedTime, called by Binder to block concurrent configuration
   */
  void OnlineController::setBlockedTime(sc_time time){
    //this->waitInterval = &time;
  }
  
   /**
   * \brief Implementation of ListBinder::getConfiguration
   * Used e.g. to the setuptime with getLoadtime()
   */
  Configuration* OnlineController::getConfiguration(ProcessControlBlock task){
    //Director* myDir = dynamic_cast<Director*>(getDirector());
    Director* myDir = &Director::getInstance(); 
    //ReconfigurableComponent* myComp = myDir->getReComp();
    std::string aReComp =
    task.getBindingGraph().getRoot()->getChildIterator()->getNext()->getID();
    ReconfigurableComponent* myComp = myDir->getCompByName(aReComp);
    

    if(myComp == NULL){
      std::cerr << "ListBinder> MyComp ist NULL" << std::endl;
    }
    AbstractController* myCtrl = myComp->getController();
    if(myCtrl == NULL){
      std::cerr << "ListBinder> MyCtrl ist NULL" << std::endl;
    }
    
    Binding* myBinding = task.getBindingGraph().getBinding(myComp->basename());
    ChildIterator* myBindingChildIter = myBinding->getChildIterator();
    Binding* myBindingChild;
    if (myBindingChildIter->hasNext()) 
      myBindingChild = myBindingChildIter->getNext();
    
    unsigned int ConfID = myCtrl->getConfigurationMapper()->getConfigForComp(myBindingChild->getID());
    
    Configuration* myConfig = myComp->getConfiguration(ConfID);
    
    return myConfig;
  }
  
  /**
   * \brief Implementation of ListBinder::getSetuptime
   */
  sc_time OnlineController::getSetuptime(ProcessControlBlock task){
    Configuration* myConfig = this->getConfiguration(task);
    sc_time setuptime = myConfig->getLoadTime();
    
    return setuptime;
  }
  
  /**
   * \brief Implementation of ListBinder::getRuntime
   */
  sc_time OnlineController::getRuntime(ProcessControlBlock task){
    
    //getReconfigurableComponent
    Binding* RecomponentBinding = task.getBindingGraph().getRoot();
    Binding* RecomponentBindingChild;
    ChildIterator* RecomponentBindingChildIter = RecomponentBinding->getChildIterator();
    if(RecomponentBindingChildIter->hasNext())
      RecomponentBindingChild = RecomponentBindingChildIter->getNext();
    delete RecomponentBindingChildIter;
    
    //getMappingInformation
    MappingInformationIterator* MapInfoIter = RecomponentBindingChild->getMappingInformationIterator();
    MappingInformation* mInfo;
    if(MapInfoIter->hasNext())
      mInfo = MapInfoIter->getNext();
    delete MapInfoIter;
    
    return mInfo->getDelay();
  }
  
} //end of Namespace SystemC_VPC
