#include <hscd_vpc_ReconfigurableComponent.h>
#include <hscd_vpc_datatypes.h>
#include <hscd_vpc_Director.h>

#include <values.h>

namespace SystemC_VPC{
  /**
   * IMPLEMENTATION OF ReconfigurableComponent
   */

  /**
   * \brief An implementation of AbstractComponent used together with passive actors and global SMoC v2 Schedulers.
   */
  ReconfigurableComponent::ReconfigurableComponent(sc_module_name name, 
                                                    AbstractController* controller)
    : AbstractComponent(name), 
      controller(NULL),
      activConfiguration(NULL),
      storeStartTime(NULL),
      remainingStoreTime(NULL),
      wakeUpSignalled(false) {

    SC_THREAD(schedule_thread);
    this->setController(controller);

#ifndef NO_VCD_TRACES
    std::string tracefilename = this->basename();
    char tracefilechar[VPC_MAX_STRING_LENGTH];
    char* traceprefix= getenv("VPCTRACEFILEPREFIX");
    if(NULL != traceprefix){
      tracefilename.insert(0,traceprefix);
    }
    strcpy(tracefilechar,tracefilename.c_str());
    this->traceFile = sc_create_vcd_trace_file(tracefilechar);
    ((vcd_trace_file*)this->traceFile)->sc_set_vcd_time_unit(-9);        
#endif //NO_VCD_TRACES

    if(!this->isActiv()){
#ifdef VPC_DEBUG
      std::cerr << RED(this->basename() << "> Activating") << std::endl;
#endif //VPC_DEBUG
      this->setActiv(true);
    }

  }

  /**
   * \brief Implementation of destructor of ReconfigurableComponent
   */
  ReconfigurableComponent::~ReconfigurableComponent(){

    delete this->controller;

  }

  /**
   * \brief Implementation of ReconfigurableComponent::schedule_thread()
   */
  void ReconfigurableComponent::schedule_thread(){
    /**
     ** set up configurations traces
     **/

    // set all traced configurations to passiv
    std::map<std::string, sc_signal<trace_value>* >::iterator iter;
    for(iter = this->trace_map_by_name.begin(); iter != this->trace_map_by_name.end(); iter++){
      *(iter->second) = S_PASSIV;
    }
    // set loaded config activ
    if(this->activConfiguration != NULL){
      iter = this->trace_map_by_name.find(this->activConfiguration->getName());
      if(iter != this->trace_map_by_name.end()){
        *(iter->second) = S_ACTIV;
      }
    }

    /*******************************************
     ** 
     ** while(true){
     **       wait for new tasks;
     **       pass Tasks to Controller;
     **       request delegatable tasks from Controller;
     **       delegate them by using informations from Controller;
     **        if new configuration requested?
     **          load configuration
     **  }
     **
     *******************************************/

    sc_time* minTimeToWait = NULL;
    bool newTasksDuringLoad = false;

    while(true){
      // if there are still tasks remaining to pass to controller dont wait for notification
      if( !newTasksDuringLoad && !wakeUpSignalled && this->isActiv() ){  
#ifdef VPC_DEBUG
        std::cerr << RED("ReconfigurableComponent "<< this->basename() <<"> going to wait at time: ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

        // check if ReconfigurableComponent has to wait for configuration to "run to completion"
        if(minTimeToWait == NULL){

#ifdef VPC_DEBUG
          std::cerr << RED("ReconfigurableComponent "<< this->basename() <<"> waiting for new tasks.") << endl;
#endif //VPC_DEBUG

          wait(this->notify_schedule_thread | this->notify_preempt);

        }else{

#ifdef VPC_DEBUG
          std::cerr << RED("ReconfigurableComponent "<< this->basename() <<"> waiting for redelegation in: ") << minTimeToWait->to_default_time_units() << endl;
#endif //VPC_DEBUG

          wait(*minTimeToWait, this->notify_schedule_thread | this->notify_preempt);
          minTimeToWait = NULL;

        }
      }else{

        newTasksDuringLoad = false;
        wakeUpSignalled = false;
        
      }  


#ifdef VPC_DEBUG
      std::cerr << RED("ReconfigurableComponent "<< this->basename() <<"> got notified at time: " << sc_simulation_time() 
          << " with num of new tasks= " << this->newTasks.size()) << endl;
#endif //VPC_DEBUG

      // if component is preempted wait till resume
      if( !this->isActiv() ){

#ifdef VPC_DEBUG
        std::cerr << RED("ReconfigurableComponent "<< this->basename() <<"> not activ going to sleep at time: ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

        this->controller->signalPreemption(this->killed, this);
        // hold pointer to currentlyloaded configuration
        Configuration* currConfig;
        if(this->killed){
          currConfig = NULL;
        }else{
          currConfig = this->activConfiguration;
        }

        // check if activ configuration has to be saved
        if(this->activConfiguration != NULL && !this->activConfiguration->isStored()){
          this->storeActivConfiguration(this->killed);
        }

        wait(this->notify_resume);

        this->loadConfiguration(currConfig);
        this->controller->signalResume(this);

#ifdef VPC_DEBUG
        std::cerr << RED("ReconfigurableComponent "<< this->basename() <<"> awoke at time: ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

      }
      
      do{
        // inform controller about new tasks
        this->controller->addTasksToSchedule(this->newTasks, this);
        // panik insure to get all tasks!
        wait(SC_ZERO_TIME);
      }while(this->newTasks.size() > 0);

      this->controller->performSchedule(this);

#ifdef VPC_DEBUG
      std::cerr << RED("ReconfigurableComponent "<< this->basename() <<"> finished delegation to controller !") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

      // delegate all processable tasks
      // points to currently viewed task to delegate
      ProcessControlBlock* currTask;
      // points to component to delegate task to
      AbstractComponent* currComp;

      while(this->controller->hasTaskToProcess(this)){

        currTask = this->controller->getNextTask(this);

#ifdef VPC_DEBUG
        std::cerr << RED("ReconfigurableComponent "<< this->basename() <<"> still task to forward: ") << currTask->getName() << " at "  << sc_simulation_time() << endl;
#endif //VPC_DEBUG


        currComp = this->controller->getMappedComponent(currTask, this);
        currComp->compute(currTask);

      }


#ifdef VPC_DEBUG
      std::cerr << RED("ReconfigurableComponent "<< this->basename() <<"> finished forwarding and checking config !") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

      // check if new configuration has to be loaded
      // points to required configuration for a given task
      Configuration* nextConfig = NULL;
      unsigned int confID = this->controller->getNextConfiguration(this);
      try{
        // if confID == 0 we have no new configuration
        if(confID != 0){
          nextConfig = this->confPool.getConfigByID(confID);
        }
      }catch(InvalidArgumentException& e){
        // ignore 
        std::cerr << e.what() << std::endl;
      }

  
      // there is new configuration load it
      if(nextConfig != NULL && nextConfig != this->activConfiguration){

#ifdef VPC_DEBUG
        std::cerr << BLUE("ReconfigurableComponent " << this->basename() << "> new config to load: ") << nextConfig->getName() << std::endl;
#endif //VPC_DEBUG

        // perform reconfiguration
        if(this->storeActivConfiguration(this->controller->preemptByKill())){
          // perform loading new configuration if no preemption happend!
          if(this->isActiv() && !this->loadConfiguration(nextConfig)){

#ifdef VPC_DEBUG
            std::cerr << BLUE("ReconfigurableComponent " << this->basename() << "> failed loading!") << std::endl;
#endif //VPC_DEBUG

          }else{

#ifdef VPC_DEBUG
            std::cerr << BLUE("ReconfigurableComponent " << this->basename() << "> new configuration loaded: " << this->activConfiguration->getName()) << std::endl;
#endif //VPC_DEBUG
          }

        }else{

#ifdef VPC_DEBUG
          std::cerr << BLUE("ReconfigurableComponent " << this->basename() << "> failed storing!") << std::endl;
#endif //VPC_DEBUG

        }

        //check if new task register during loadphase which may have caused wait
        if(this->newTasks.size() > 0){

#ifdef VPC_DEBUG
          std::cerr << RED("New tasks during configuration load!") << std::endl;
#endif //VPC_DEBUG

          newTasksDuringLoad = true;
        }
      }
      
      // check if controller request special interval to be called next time
      minTimeToWait = this->controller->getWaitInterval(this);

    }
  }

  /**
   * \brief Implementation of ReconfigurableComponent::addConfiguration
   */
  void ReconfigurableComponent::addConfiguration(const char* name, Configuration* config){

    assert(name != NULL);
    assert(config != NULL);

#ifndef NO_VCD_TRACES
    sc_signal<trace_value>* newsignal = new sc_signal<trace_value>();
    trace_map_by_name.insert(pair<string,sc_signal<trace_value>*>(name, newsignal));
    sc_trace(this->traceFile, *newsignal, name);
#endif //NO_VCD_TRACES

    //this->config_map_by_name[name] = config;
    this->confPool.addConfiguration(config);
   
  }

  /**
   * \brief Implementation of ReconfigurableComponent::getConfiguration
   */
  Configuration* ReconfigurableComponent::getConfiguration(unsigned int id){

    try{
      return this->confPool.getConfigByID(id);
    }catch(InvalidArgumentException& e){
      std::cerr << e.what() << std::endl;
    }
    /*
    map<std::string, Configuration* >::iterator iter;

    iter = this->config_map_by_name.find(name);
    if(iter != this->config_map_by_name.end()){
      return iter->second;
    }
    */
    return NULL;

  }

  /**
   * \brief Implementation of ReconfigurableComponent::getConfigurations
   */
  /*
  std::map<std::string, Configuration*>& ReconfigurableComponent::getConfigurations(){

    return this->config_map_by_name;

  }*/

  /**
   * \brief Implementation of ReconfigurableComponent::getConfigurationPool
   */
  ConfigurationPool& ReconfigurableComponent::getConfigurationPool(){

    return this->confPool;

  }
  
  /**
   * \brief Implementation of ReconfigurableComponent::getActivConfiguraton
   */
  Configuration* ReconfigurableComponent::getActivConfiguration(){

    return this->activConfiguration;

  }

  /**
   * \brief Implementation of ReconfigurableComponent::setActivConfiguration
   */
  void ReconfigurableComponent::setActivConfiguration(unsigned int name){

    assert(name != 0);

    try{
      
      this->activConfiguration = this->confPool.getConfigByID(name);

      // activate new configration
      this->activConfiguration->resume();

#ifdef VPC_DEBUG
      std::cerr << "ReconfigurableComponent> activ Configuration " << this->activConfiguration->getName() << " is activ "
        << this->activConfiguration->isActiv() << std::endl;
#endif //VPC_DEBUG

    }catch(InvalidArgumentException& e){
      std::cerr << "ReconfigurableComponent> Unable to set activ configuration to " << name << " due to " << e.what() << std::endl;
    }

#ifdef VPC_DEBUG
    std::cerr << "ReconfigurableComponent> new activ Configuration is " << this->activConfiguration->getName() << " with id " << this->activConfiguration->getID() << std::endl;
#endif //VPC_DEBUG
  }

  /**
   * \brief Implementation of ReconfigurableComponent::getController
   */
  AbstractController* ReconfigurableComponent::getController(){

    return this->controller;

  }

  /**
   * \brief Implementation of ReconfigurableComponent::setController
   */
  void ReconfigurableComponent::setController(AbstractController* controller){

    assert(controller != NULL);

    this->controller = controller;
    // register backward to controller
    this->controller->setManagedComponent(this);
  } 

  /**
   * \brief Implementation of ReconfigurableComponent::preempt
   */
  void ReconfigurableComponent::preempt(bool kill){

    // only preempt activ component
    if(this->isActiv()){
      this->killed = kill;
      this->setActiv(false);
      this->notify_preempt.notify();   
    }

  }

  /**
   * \brief Implementation of ReconfigurableComponent::resume
   */
  void ReconfigurableComponent::resume(){

    // only resume preempted component
    if(!this->isActiv()){
      this->killed = false;
      this->setActiv(true);
      this->notify_resume.notify();  
    }

  }

  /**
   * \brief Implementation of ReconfigurableComponent::timeToPreempt
   */  
  sc_time ReconfigurableComponent::timeToPreempt(){
    sc_time time(SC_ZERO_TIME);

    // only if component is activ we have time for preemption
    if(this->isActiv()){
      // if component is in storing phase
      if(this->storeStartTime != NULL && this->remainingStoreTime != NULL){
        time += *(this->remainingStoreTime) - (sc_time_stamp() - *(this->storeStartTime));
      }else
        if(this->activConfiguration != NULL){
          time = this->activConfiguration->timeToPreempt();
          time += this->activConfiguration->getStoreTime();
        }
    }

    return time;
  }

  /**
   * \brief Implementation of ReconfigurableComponent::timeToResume
   */  
  sc_time ReconfigurableComponent::timeToResume(){

    sc_time time(SC_ZERO_TIME);

    // only if component is inactiv we have time to resume
    if(!this->isActiv() && this->activConfiguration != NULL){

      time = this->activConfiguration->timeToResume();
      time += this->activConfiguration->getLoadTime();

    }

    return time;

  }

  /**
   * \brief An implementation of AbstractComponent::_compute(const char *, const char *, VPC_Event).
   */
  void ReconfigurableComponent::_compute( const char* name, const char* funcname, VPC_Event* end){

    // send compute request to controller
    ProcessControlBlock* pcb = Director::getInstance().getProcessControlBlock(name);
    pcb->setBlockEvent(end);
    this->compute(pcb);

  }

  /**
   * \brief An implementation of AbstractComponent::_compute(const char *, VPC_Event).
   */
  void ReconfigurableComponent::_compute( const char *name, VPC_Event *end){

    this->_compute(name, "", end);

  }

  /**
   * \brief An implementation of AbstractComponent::compute(ProcessControlBlock*, const char *).
   */
  void ReconfigurableComponent::compute(ProcessControlBlock* pcb){

    //std::cerr << "ReconfigurableComponent " << this->basename() << "> compute(" << pcb->getName() << ":" << pcb->getFuncName() << " at " << sc_simulation_time() << std::endl;
    this->newTasks.push_back(pcb);

    this->notify_schedule_thread.notify();

  }

  /**
   * \brief Used to create the Tracefiles.
   *
   * To create a vcd-trace-file in SystemC all the signals to 
   * trace have to be in a "global" scope. The signals have to 
   * be created in elaboration phase (before first sc_start).
   */
  void ReconfigurableComponent::informAboutMapping(std::string module){
    // TODO IMPLEMENT
  }

  /**
   * \brief Set parameter for Component and Scheduler.
   */
  void ReconfigurableComponent::processAndForwardParameter(char *sType,char *sValue){

    if(this->controller != NULL){

      this->controller->setProperty(sType, sValue);

    }

  }

  bool ReconfigurableComponent::storeActivConfiguration(bool kill){

    // remember start time of storing
    sc_time storeStart = sc_time_stamp();
    sc_time timeToStore = SC_ZERO_TIME;
    // "mark" loading phase
    this->storeStartTime = &storeStart;
    this->remainingStoreTime = &timeToStore;

    // stop currently running components
    if(this->activConfiguration != NULL){

#ifdef VPC_DEBUG
      std::cerr << "ReconfigurableComponent" << this->basename() << "> trying to store config kill=" << kill << std::endl;
#endif //VPC_DEBUG


#ifndef NO_VCD_TRACES
      this->traceConfigurationState(this->activConfiguration, S_CONFIG);
#endif //NO_VCD_TRACES

      //update time for reconfiguration if storing is required
      if(!kill){
        sc_time time = this->activConfiguration->timeToPreempt();
        timeToStore += time;
      }

      this->activConfiguration->preempt(kill);


      // Simulate store time if required
      if(!kill){
        //update time for reconfiguration
        timeToStore += this->activConfiguration->getStoreTime();
      }

      // perform simulating storing as long as time not elapsed and no preemption by kill happend
      while(timeToStore > SC_ZERO_TIME){

        wait(timeToStore, this->notify_preempt);

        timeToStore -= sc_time_stamp() - storeStart;
        storeStart = sc_time_stamp();

        //check if preemption happend with kill!
        if(this->killed){

#ifdef VPC_DEBUG
          std::cerr << "ReconfigurableComponent " << this->basename() << "> storing configuration has been interrupted " << std::endl;
#endif //VPC_DEBUG

          this->activConfiguration->setStored(false);

#ifndef NO_VCD_TRACES
          this->traceConfigurationState(this->activConfiguration, S_PASSIV);
#endif //NO_VCD_TRACES

          this->activConfiguration = NULL;

          // "mark" end of loading phase
          this->storeStartTime = NULL;
          this->remainingStoreTime = NULL;

          // signal preemption to controller instance
          this->controller->signalPreemption(this->killed, this);
          
          return false;

        }    
      }

      this->activConfiguration->setStored(true);

#ifndef NO_VCD_TRACES
      this->traceConfigurationState(this->activConfiguration, S_PASSIV);
#endif //NO_VCD_TRACES

      this->activConfiguration = NULL;          
    }

    // "mark" end of loading phase
    this->storeStartTime = NULL;
    this->remainingStoreTime = NULL;

    return true;
  }

  bool ReconfigurableComponent::loadConfiguration(Configuration* config){

    //remember start time of storing
    sc_time loadStart = sc_time_stamp();
    sc_time timeToLoad = SC_ZERO_TIME;

    //load new configuration
    if(config != NULL){

#ifdef VPC_DEBUG
      std::cerr << "ReconfigurableComponent " << this->basename() << "> loading configuration config= " << config->getName() << std::endl;
#endif //VPC_DEBUG

#ifndef NO_VCD_TRACES
      this->traceConfigurationState(config, S_CONFIG);
#endif //NO_VCD_TRACES

      //simulate loading time
      timeToLoad += config->getLoadTime();
      wait(config->getLoadTime(), this->notify_preempt);

      // check if preemption happened
      if(reconfigurationInterrupted(loadStart, timeToLoad)){
        this->activConfiguration = NULL;

#ifndef NO_VCD_TRACES
        this->traceConfigurationState(config, S_PASSIV);
#endif //NO_VCD_TRACES

        return false;
      }

      sc_time time = config->timeToResume(); 
      timeToLoad += time;


      config->resume(); 
      // wait time of resume
      wait(time, this->notify_preempt);

      // check if preemption happened
      if(reconfigurationInterrupted(loadStart, timeToLoad)){
        // return with unresumed configuration -> should be already preempted again
        this->activConfiguration = NULL;

        // signal preemption to controller instance
        this->controller->signalPreemption(this->killed, this);
        
#ifndef NO_VCD_TRACES
        this->traceConfigurationState(config, S_PASSIV);
#endif //NO_VCD_TRACES

        return false;
      }

      // mark configuration as loaded
      config->setStored(false);    
      this->activConfiguration = config;

#ifndef NO_VCD_TRACES
      this->traceConfigurationState(config, S_ACTIV);
#endif //NO_VCD_TRACES

    }  

    return true;
  }

  bool ReconfigurableComponent::reconfigurationInterrupted(sc_time timeStamp, sc_time interval){

    //check if preemption happend
    sc_time elapsedTime = sc_time_stamp() - timeStamp;

    if(elapsedTime.value() < interval.value()){

#ifdef VPC_DEBUG
      std::cerr << "ReconfigurableComponent " << this->basename() << "> reconfiguration phase has been interrupted " << std::endl;
      std::cerr << "ReconfigurableComponent " << this->basename() << "> startTime= " << timeStamp << " interval= " << interval
        << " elapsedTime= " << elapsedTime << std::endl;
#endif //VPC_DEBUG

      return true;

    }

    return false;
  }

  void ReconfigurableComponent::traceConfigurationState(Configuration* config, trace_value value){

    if(1==trace_map_by_name.count(config->getName())){
      std::map<std::string, sc_signal<trace_value>* >::iterator iter = trace_map_by_name.find(config->getName());
      *(iter->second) = value;
    }

  }

}//namespace SystemC_VPC
